#include <bits/stdc++.h>

using namespace std;

struct edge
{
	int adj,w;
};

int main(int argc, char *argv[])
{
	ios_base::sync_with_stdio(false);
	cin.tie(0);
	
	fstream fin, fout;
	fin.open(argv[1], ios::in | ios::binary);
	fout.open(argv[2], ios::out);
	int numProc = atoi(argv[3]);
	
	int N, M;
	fin.read(reinterpret_cast<char *>(&N), sizeof(N));
	fin.read(reinterpret_cast<char *>(&M), sizeof(M));
	
	int **G;
	
	G = new int*[N];
	
	for(int i=0 ; i<N ; ++i)
		G[i] = new int[N];
	
	for(int i=0 ; i<N ; ++i)
		for(int j=0 ; j<N ; ++j)
			G[i][j] = 0;
	
	for(int i=0 ; i<M ; ++i)
	{
		int u, v, w;
		fin.read(reinterpret_cast<char *>(&u), sizeof(u));
		fin.read(reinterpret_cast<char *>(&v), sizeof(v));
		fin.read(reinterpret_cast<char *>(&w), sizeof(w));
		G[u][v] = G[v][u] = 1;
	}
	fin.close();
	
	set<int> part[numProc];
	int curPart[N], maxN = N*3/2/numProc;
	
	for(int i=0 ; i<N ; ++i)
	{
		part[i%numProc].insert(i);
		curPart[i] = i%numProc;
	}
	
	int edgeNum[N][numProc];

	for(int i=0 ; i<N ; ++i)
		for(int j=0 ; j<numProc ; ++j)
			edgeNum[i][j] = 0;
	
	for(int i=0 ; i<N ; ++i)
		for(int adj=0 ; adj<N ; ++adj)
			if(G[i][adj])
				for(int j=0 ; j<numProc ; ++j)
					if(part[j].count(adj))
						edgeNum[i][j]++;
	
	while(1)
	{
		int id = -1, to, gain = 0;
		for(int i=1 ; i<N ; ++i)
		{
			for(int j=0 ; j<numProc ; ++j)
			{
				if(part[j].size()<maxN)
				{
					int curGain = edgeNum[i][j] - edgeNum[i][curPart[i]];
					if(curGain > gain)
					{
						gain = curGain;
						id = i;
						to = j;
					}
				}
			}
		}
		
		if(id==-1)
			break;
		
		int prev = curPart[id];
		part[curPart[id]].erase(part[curPart[id]].find(id));
		part[to].insert(id);
		curPart[id] = to;
		
		for(int i=0 ; i<N ; ++i)
		{
			if(G[i][id])
			{
				edgeNum[i][prev]--;
				edgeNum[i][to]++;
			}
		}
	}
	
	for(int i=0 ; i<N ; ++i)
		fout << curPart[i] << endl;
	
	fout.close();
	return 0;
}