#include <bits/stdc++.h>
#include <mpi.h>

using namespace std;

struct edge
{
	int adj, w;
};

int main(int argc, char *argv[])
{
	ios_base::sync_with_stdio(false);
	cin.tie(0);
	
	MPI_Init(&argc, &argv);
	int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	fstream fin;
	fin.open(argv[1], ios::in | ios::binary);
	
	int N, M;
	vector<int> vID;
	vector<edge> *wVec;
	
	fin.read(reinterpret_cast<char *>(&N), sizeof(N));
	fin.read(reinterpret_cast<char *>(&M), sizeof(M));
	
	int num = 0;
	int dest[N], f[N];
	
	if(argc==4) // with partition
	{
		fstream fpar;
		fpar.open(argv[3], ios::in);
		for(int i=0 ; i<N ; ++i)
		{
			fpar >> dest[i];
			if(dest[i]==rank)
			{
				vID.push_back(i);
				f[i] = num++;
			}
		}
	}
	else
	{
		for(int i=0 ; i<N ; ++i)
		{
			dest[i] = i%size;
			if(dest[i]==rank)
			{
				vID.push_back(i);
				f[i] = num++;
			}
		}
	}
	
	wVec = new vector<edge>[vID.size()];
	for(int i=0 ; i<M ; ++i)
	{
		int u, v, w;
		fin.read(reinterpret_cast<char *>(&u), sizeof(u));
		fin.read(reinterpret_cast<char *>(&v), sizeof(v));
		fin.read(reinterpret_cast<char *>(&w), sizeof(w));
		if(dest[u]==rank && u!=v && v!=0)
		{
			edge e;
			e.adj = v;
			e.w = w;
			wVec[f[u]].push_back(e);
		}
	}
	fin.close();
	
	int ans[vID.size()];
	
	if(size==1)
	{
		queue<int> qu;
		bool inq[vID.size()];
		for(int i=0 ; i<vID.size() ; ++i)
		{
			ans[i] = INT_MAX;
			inq[i] = false;
		}
		ans[0] = 0;
		qu.push(0);
		inq[0] = true;
		while(!qu.empty())
		{
			int cur = qu.front();
			qu.pop();
			inq[cur] = false;
			for(edge e : wVec[cur])
			{
				int relax = ans[cur]+e.w;
				if(relax < ans[f[e.adj]])
				{
					ans[f[e.adj]] = relax;
					if(!inq[f[e.adj]])
					{
						qu.push(f[e.adj]);
						inq[f[e.adj]] = true;
					}
				}
			}
		}
	}
	else
	{
		bool inq[vID.size()], needSend = false, here = false, done = false;
		int recv[size][N], send[N], color = 0, prev = (size+rank-1)%size, next = (rank+1)%size;
		MPI_Request req, reqR[size];
		queue<int> qu;
		
		for(int i=0 ; i<vID.size() ; ++i)
		{
			inq[i] = false;
			ans[i] = INT_MAX;
		}
	
		for(int i=0 ; i<N ; ++i)
			send[i] = INT_MAX;
	
		if(rank == 0)
		{
			ans[0] = 0;
			qu.push(0);
			inq[0] = true;
			here = true;
		}
	
		if(!here)
			MPI_Irecv(&color, 1, MPI_INT, prev, size, MPI_COMM_WORLD, &req);
	
		for(int i=0 ; i<size ; ++i)
			if(rank!=i)
				MPI_Irecv(recv[i], N, MPI_INT, i, 0, MPI_COMM_WORLD, &reqR[i]);

		while(!done)
		{
			for(int i=0 ; i<size ; ++i)
			{
				if(rank!=i)
				{
					int flag;
					MPI_Test(&reqR[i], &flag, MPI_STATUS_IGNORE);
					if(flag)
					{
						for(int j=0 ; j<vID.size() ; ++j)
						{
							if(recv[i][vID[j]] < ans[j])
							{
								ans[j] = recv[i][vID[j]];
								if(!inq[j])
								{
									qu.push(j);
									inq[j] = true;
								}
							}
						}
						MPI_Irecv(recv[i], N, MPI_INT, i, 0, MPI_COMM_WORLD, &reqR[i]);
					}
				}
			}
			if(!qu.empty())	
			{
				if(!here)
				{
					int flag;
					MPI_Test(&req, &flag, MPI_STATUS_IGNORE);
					if(flag)
						here = true;
				}
				if(here)
					color = 0;
				while(!qu.empty())
				{
					int cur = qu.front();
					qu.pop();
					inq[cur] = false;
					for(edge e : wVec[cur])
					{
						int relax = ans[cur]+e.w;
						if(rank == dest[e.adj])
						{
							if(relax < ans[f[e.adj]])
							{
								ans[f[e.adj]] = relax;
								if(!inq[f[e.adj]])
								{
									qu.push(f[e.adj]);
									inq[f[e.adj]] = true;
								}
							}
						}
						else
						{
							if(relax < send[e.adj])
							{
								needSend = true;
								send[e.adj] = relax;
							}
						}
					}
				}
			}
			else
			{
				if(needSend)
				{
					for(int i=0 ; i<size ; ++i)
						if(rank!=i)
							MPI_Send(send, N, MPI_INT, i, 0, MPI_COMM_WORLD);
					needSend = false;
				}
				else
				{
					if(here)
					{
						if(rank==0)
							color++;
						MPI_Send(&color, 1, MPI_INT, next, size, MPI_COMM_WORLD);
						here = false;
						if(color==5)
							done = true;
						else
							MPI_Irecv(&color, 1, MPI_INT, prev, size, MPI_COMM_WORLD, &req);
					}
					else
					{
						int flag;
						MPI_Test(&req, &flag, MPI_STATUS_IGNORE);
						if(flag)
							here = true;
					}
				}
			}
		}
	}
	
	MPI_File fout;
	MPI_File_open(MPI_COMM_WORLD, argv[2], MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, &fout);
	for(int i=0 ; i<vID.size() ; ++i)
		MPI_File_write_at(fout, vID[i]*sizeof(int), &ans[i], 1, MPI_INT, MPI_STATUS_IGNORE);
	MPI_File_close(&fout);
	
	MPI_Finalize();
	return 0;
}