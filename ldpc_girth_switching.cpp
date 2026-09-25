#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <queue>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;
using EdgeKey = uint64_t;

struct Graph {
    int nU, nW, N, dv, dc;
    vector<vector<int>> adj;
    vector<unordered_set<int>> nbr;

    Graph(int n, int a, int b)
        : nU(n), nW(n*a/b), N(n+nW), dv(a), dc(b), adj(N), nbr(N) {}

    bool has(int x, int y) const { return nbr[x].count(y) != 0; }

    void add(int x, int y) {
        adj[x].push_back(y); adj[y].push_back(x);
        nbr[x].insert(y); nbr[y].insert(x);
    }

    void del(int x, int y) {
        nbr[x].erase(y); nbr[y].erase(x);
        auto &a=adj[x]; *find(a.begin(),a.end(),y)=a.back(); a.pop_back();
        auto &b=adj[y]; *find(b.begin(),b.end(),x)=b.back(); b.pop_back();
    }

    EdgeKey key(int x, int y) const {
        if (x>y) swap(x,y);
        return (EdgeKey)(uint32_t)x<<32 | (uint32_t)y;
    }
};

/* A deterministic simple biregular initializer, randomized by valid
   degree-preserving 2-switches using the supplied seed. */
Graph make_input(int n, int dv, int dc, uint64_t seed) {
    if ((long long)n*dv % dc) throw runtime_error("n*dv must be divisible by dc");
    int nw=n*dv/dc;
    Graph g(n,dv,dc);
    for (int u=0;u<n;++u)
        for (int j=0;j<dv;++j)
            g.add(u,n+((u*dv+j)%nw));

    mt19937_64 rng(seed);
    uniform_int_distribution<int> pickU(0,n-1);
    const long long rounds=20LL*n*dv;
    for (long long it=0;it<rounds;++it) {
        int u=pickU(rng), v=pickU(rng);
        if (u==v) continue;
        int w=g.adj[u][rng()%g.adj[u].size()];
        int z=g.adj[v][rng()%g.adj[v].size()];
        if (w==z || g.has(u,z) || g.has(v,w)) continue;
        g.del(u,w); g.del(v,z);
        g.add(u,z); g.add(v,w);
    }
    return g;
}

int switching_target(int n, int dv, int dc) {
    long long q=1LL*(dv-1)*(dc-1);
    long double x=(long double)n*(q-1)/(2.0L*dc), p=1;
    int k=0;
    while (p*q<=x) { p*=q; ++k; }
    return 2*k+2;
}

struct Cycle {
    vector<EdgeKey> edges;
    bool alive=true;
};

/* Enumerate every simple cycle of a prescribed even length exactly once.
   Starting at the smallest U-vertex and fixing one orientation removes
   rotational and reversal duplicates. */
vector<Cycle> cycles_of_length(Graph& g, int L) {
    vector<Cycle> cycles;
    vector<int> path(L+1), seen(g.N,0);
    int token=0;

    function<void(int,int,int)> dfs = [&](int s,int x,int depth) {
        if (depth==L) {
            if (x!=s || path[1]>=path[L-1]) return;
            vector<EdgeKey> es;
            es.reserve(L);
            for (int i=0;i<L;++i) es.push_back(g.key(path[i],path[i+1]));
            sort(es.begin(),es.end());
            cycles.push_back({move(es),true});
            return;
        }

        for (int y:g.adj[x]) {
            if (depth==L-1) {
                if (y!=s) continue;
            } else {
                if (y==s || seen[y]==token) continue;
                if (y<g.nU && y<s) continue;
            }
            path[depth+1]=y;
            if (y!=s) seen[y]=token;
            dfs(s,y,depth+1);
            if (y!=s) seen[y]=0;
        }
    };

    for (int s=0;s<g.nU;++s) {
        ++token;
        path[0]=s; seen[s]=token;
        dfs(s,s,0);
        seen[s]=0;
    }
    return cycles;
}

struct BFSWork {
    vector<int> mark, dist, q;
    int token=1;
    explicit BFSWork(int N):mark(N),dist(N),q(N){}
};

/* Find the first remote edge in the deterministic scan order.
   The BFS computes distance from the two endpoints of e. */
pair<int,int> remote_edge(Graph& g,int eu,int ew,int girth,BFSWork& W) {
    ++W.token;
    int head=0,tail=0;
    W.q[tail++]=eu; W.mark[eu]=W.token; W.dist[eu]=0;
    W.q[tail++]=ew; W.mark[ew]=W.token; W.dist[ew]=0;

    while (head<tail) {
        int x=W.q[head++], dx=W.dist[x];
        if (dx>=girth-1) continue;
        for (int y:g.adj[x])
            if (W.mark[y]!=W.token) {
                W.mark[y]=W.token;
                W.dist[y]=dx+1;
                W.q[tail++]=y;
            }
    }

    for (int u=0;u<g.nU;++u)
        for (int w:g.adj[u]) {
            bool farU=(W.mark[u]!=W.token || W.dist[u]>=girth);
            bool farW=(W.mark[w]!=W.token || W.dist[w]>=girth);
            if (farU && farW) return {u,w};
        }
    return {-1,-1};
}

int main(int argc,char**argv) {
    int n=1944,dv=3,dc=6;
    uint64_t seed=1;
    for (int i=1;i<argc;++i) {
        string x=argv[i];
        if (x=="--n" && i+1<argc) n=stoi(argv[++i]);
        else if (x=="--dv" && i+1<argc) dv=stoi(argv[++i]);
        else if (x=="--dc" && i+1<argc) dc=stoi(argv[++i]);
        else if (x=="--seed" && i+1<argc) seed=stoull(argv[++i]);
    }

    auto start=chrono::steady_clock::now();
    Graph g=make_input(n,dv,dc,seed);
    BFSWork work(g.N);
    const int target=switching_target(n,dv,dc);

    int gin=target;
    long long switches=0;

    /* At a fixed current girth g, the Safe Remote Switch lemma implies
       that no new cycle of length <=g is created. Therefore the initially
       enumerated g-cycles can be maintained exactly: a surviving g-cycle
       disappears iff one of its edges is deleted. This is an implementation
       of the paper's maximum-incidence rule, not a heuristic. */
    for (int L=4;L<target;L+=2) {
        auto cycles=cycles_of_length(g,L);
        if (!cycles.empty() && gin==target) gin=L;

        while (true) {
            unordered_map<EdgeKey,int> incidence;
            int alive=0;
            for (auto &c:cycles) if (c.alive) {
                ++alive;
                for (EdgeKey e:c.edges) ++incidence[e];
            }
            if (!alive) break;

            EdgeKey best=0;
            int bestCount=-1;
            for (const auto &kv:incidence)
                if (kv.second>bestCount ||
                    (kv.second==bestCount && kv.first<best)) {
                    bestCount=kv.second; best=kv.first;
                }

            int u=(int)(best>>32), w=(int)(uint32_t)best;
            if (u>=g.nU) swap(u,w);

            auto f=remote_edge(g,u,w,L,work);
            if (f.first<0) throw runtime_error("remote edge not found");
            int v=f.first, z=f.second;
            EdgeKey fk=g.key(v,z);

            for (auto &c:cycles) if (c.alive) {
                if (binary_search(c.edges.begin(),c.edges.end(),best) ||
                    binary_search(c.edges.begin(),c.edges.end(),fk))
                    c.alive=false;
            }

            g.del(u,w); g.del(v,z);
            g.add(u,z); g.add(v,w);
            ++switches;
        }
    }

    if (gin==target) gin=target;
    double seconds=chrono::duration<double>(
        chrono::steady_clock::now()-start).count();

    cout<<"n,dv,dc,seed,g_in,g_sw,g_out,switches,time_s\n";
    cout<<n<<","<<dv<<","<<dc<<","<<seed<<","<<gin<<","<<target<<","
        <<target<<","<<switches<<","<<fixed<<setprecision(6)<<seconds<<"\n";
}
