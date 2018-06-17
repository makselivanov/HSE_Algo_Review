#include <iostream>
#include <vector>

using namespace std;

struct Edge {
    int u, c, w, rev, f, ind;
    Edge(int u, int c, int w, int r, int ind) : u(u), c(c), w(w), rev(r), f(0), ind(ind) {}
};

const int MAXN = 200;
const long long INF = 1000LL * 1000 * 1000 * 1000;
int n, m, k;
long long len[MAXN], fi[MAXN];
vector<Edge> graph[MAXN];
pair<int, int> pref[MAXN];

void dijktra() {
    fill(len, len + n, INF);
    len[0] = 0;
    vector<bool> used(n, false);
    for (int i = 0; i < n - 1; ++i) {
        long long mn = INF;
        int v = -1;
        for (int j = 0; j < n; ++j) {
            if (!used[j] && mn > len[j]) {
                v = j;
                mn = len[j];
            }
        }
        if (v == -1)
            break;
        used[v] = true;
        for (int i = 0; i < graph[v].size(); ++i) {
            const Edge& e = graph[v][i];
            if (e.c - e.f > 0 && len[e.u] > len[v] + e.w + fi[v] - fi[e.u]) {
                len[e.u] = len[v] + e.w + fi[v] - fi[e.u];
                pref[e.u] = {v, i};
            }
        }
    }
}


int main() {
    cin >> n >> m >> k;
    for (int i = 0; i < m; ++i) {
        int u, v, w;
        cin >> u >> v >> w;
        --u;
        --v;
        int indu = graph[u].size(), indv = graph[v].size();
        graph[u].push_back({v, 1, w, indv + 1, i});
        graph[v].push_back({u, 1, w, indu + 1, i});
        graph[v].push_back({u, 0, -w, indu, i});
        graph[u].push_back({v, 0, -w, indv, i});
    }
    fill(pref, pref + n, make_pair(-1, -1));
    dijktra();
    int f = 0;
    while (len[n - 1] < INF && f < k) {
        int v = n - 1;
        while (v > 0) {
            int i = pref[v].second;
            int u = pref[v].first;
            ++graph[u][i].f;
            --graph[v][graph[u][i].rev].f;
            v = u;
        }
        ++f;
        for (int i = 0; i < n; ++i)
            fi[i] += len[i];
        dijktra();
    }
    if (f < k) {
        cout << "-1\n";
        return 0;
    }
    vector<vector<int>> travel;
    long long res = 0;
    for (int i = 0; i < k; ++i) {
        int v = n - 1;
        travel.push_back({});
        while (v > 0) {
            for (int i = 1; i < graph[v].size(); i += 2) {
                if (graph[v][i].f == -1 && graph[v][i - 1].f == 0) {
                    travel.back().push_back(graph[v][i].ind);
                    res -= graph[v][i].w;
                    ++graph[v][i].f;
                    --graph[graph[v][i].u][graph[v][i].rev].f;
                    v = graph[v][i].u;
                    break;
                }
            }
        }
    }
    cout.precision(20);
    cout << static_cast<double>(res) / k << '\n';
    for (const vector<int>& vec : travel) {
        cout << vec.size() << ' ';
        for (int i = (int)vec.size() - 1; i >= 0; --i)
            cout << vec[i] + 1 << ' ';
        cout << '\n';
    }
}
