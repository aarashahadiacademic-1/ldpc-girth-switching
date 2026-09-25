#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

struct Graph {
    int nU, nW, dv, dc;
    vector<vector<int>> uadj, wadj;
    vector<unordered_set<int>> uset;
    Graph(int n,int a,int b):nU(n),nW(n*a/b),dv(a),dc(b),
        uadj(n),wadj(nW),uset(n){}
    bool has(int u,int w) const { return uset[u].count(w); }
    void add(int u,int w){ uadj[u].push_back(w); wadj[w].push_back(u); uset[u].insert(w); }
    void erase_edge(int u,int w){
        uset[u].erase(w);
        auto &a=uadj[u]; a.erase(find(a.begin(),a.end(),w));
        auto &b=wadj[w]; b.erase(find(b.begin(),b.end(),u));
    }
};

static Graph random_regular(int n,int dv,int dc,uint64_t seed){
    if((long long)n*dv%dc) throw runtime_error("n*dv must be divisible by dc");
    mt19937_64 rng(seed);
    for(int attempt=0;attempt<10000;++attempt){
        Graph G(n,dv,dc);
        vector<int> L,R;
        L.reserve(n*dv); R.reserve(n*dv);
        for(int u=0;u<n;++u) for(int j=0;j<dv;++j) L.push_back(u);
        for(int w=0;w<G.nW;++w) for(int j=0;j<dc;++j) R.push_back(w);
        shuffle(R.begin(),R.end(),rng);
        bool ok=true;
        for(size_t i=0;i<L.size();++i){
            int u=L[i],w=R[i];
            if(G.has(u,w)){ok=false;break;}
            G.add(u,w);
        }
        if(ok) return G;
    }
    throw runtime_error("failed to generate a simple regular graph; try another seed");
}

static int girth(const Graph& G){
    int N=G.nU+G.nW, best=numeric_limits<int>::max();
    vector<vector<int>> a(N);
    for(int u=0;u<G.nU;++u) for(int w:G.uadj[u]){
        int v=G.nU+w; a[u].push_back(v); a[v].push_back(u);
    }
    vector<int>d(N),p(N);
    for(int s=0;s<N;++s){
        fill(d.begin(),d.end(),-1); fill(p.begin(),p.end(),-1);
        queue<int> q; q.push(s); d[s]=0;
        while(!q.empty()){
            int x=q.front();q.pop();
            if(2*d[x]+1>=best) continue;
            for(int y:a[x]){
                if(d[y]<0){d[y]=d[x]+1;p[y]=x;q.push(y);}
                else if(p[x]!=y) best=min(best,d[x]+d[y]+1);
            }
        }
    }
    return best==numeric_limits<int>::max()?0:best;
}

static int target_g(int n,int dv,int dc){
    long long q=1LL*(dv-1)*(dc-1);
    long double x=(long double)n*(q-1)/(2.0L*dc);
    int k=0; long double z=1;
    while(z*q<=x){z*=q;++k;}
    return 2*k+2;
}

static bool try_switch(Graph& G,int target,long long& accepted){
    int oldg=girth(G);
    // Deterministic lexicographic scan. A switch (u,a),(v,b)->(u,b),(v,a)
    // is accepted only if simple and does not decrease girth.
    for(int u=0;u<G.nU;++u){
        auto au=G.uadj[u];
        for(int a:au) for(int v=u+1;v<G.nU;++v){
            auto av=G.uadj[v];
            for(int b:av){
                if(a==b || G.has(u,b) || G.has(v,a)) continue;
                G.erase_edge(u,a); G.erase_edge(v,b);
                G.add(u,b); G.add(v,a);
                int ng=girth(G);
                if(ng>=oldg){
                    ++accepted;
                    if(ng>oldg || ng>=target) return true;
                }
                // Undo switches that do not make strict progress.
                G.erase_edge(u,b); G.erase_edge(v,a);
                G.add(u,a); G.add(v,b);
                if(ng>=oldg) --accepted;
            }
        }
    }
    return false;
}

int main(int argc,char**argv){
    int n=1944,dv=3,dc=6; uint64_t seed=1;
    for(int i=1;i<argc;++i){
        string s=argv[i];
        if(s=="--n"&&i+1<argc)n=stoi(argv[++i]);
        else if(s=="--dv"&&i+1<argc)dv=stoi(argv[++i]);
        else if(s=="--dc"&&i+1<argc)dc=stoi(argv[++i]);
        else if(s=="--seed"&&i+1<argc)seed=stoull(argv[++i]);
    }
    auto t0=chrono::steady_clock::now();
    Graph G=random_regular(n,dv,dc,seed);
    int gin=girth(G), target=target_g(n,dv,dc);
    long long sw=0;
    while(girth(G)<target){
        if(!try_switch(G,target,sw)) break;
    }
    int gout=girth(G);
    double sec=chrono::duration<double>(chrono::steady_clock::now()-t0).count();
    cout<<"n,dv,dc,seed,g_in,g_sw,g_out,switches,time_s\n";
    cout<<n<<","<<dv<<","<<dc<<","<<seed<<","<<gin<<","<<target<<","
        <<gout<<","<<sw<<","<<sec<<"\n";
}
