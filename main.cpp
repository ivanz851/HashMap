#pragma GCC optimize("-O3,no-stack-protector")
// #pragma GCC target("sse,sse2,sse3,sse3,sse4")
#pragma GCC optimize("unroll-loops")
// #pragma GCC optimize("fast-math")
// #pragma GCC target("avx2")

#include <bits/stdc++.h>


using namespace std;


#include <list>
#include <stdexcept>
#include <vector>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
public:
    using iterator = typename std::list<std::pair<const KeyType, ValueType>>::iterator;
    using const_iterator = typename std::list<std::pair<const KeyType, ValueType>>::const_iterator;

    void clear() {
        Size = 0;
        Data.clear();
        Buckets.assign(DEFAULT_AMOUNT_OF_BUCKETS, Bucket());
    }

    explicit HashMap(Hash HashFunction_ = Hash()) : HashFunction(HashFunction_) {
        clear();
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> init, Hash HashFunction_ = Hash()) : HashFunction(HashFunction_) {
        clear();
        for (const auto& i : init) {
            insert(i);
        }
    }

    template<typename InitIterator>
    HashMap(InitIterator InitBegin, InitIterator InitEnd, Hash HashFunction_ = Hash()) : HashFunction(HashFunction_) {
        clear();
        while (InitBegin != InitEnd) {
            insert(*InitBegin++);
        }
    }

    HashMap& operator=(HashMap other) {
        clear();
        for (const auto& element : other) {
            this->insert(element);
        }
        return *this;
    }

    Hash hash_function() const {
        return HashFunction;
    }

    iterator begin() {
        return Data.begin();
    }
    iterator end() {
        return Data.end();
    }

    const_iterator begin() const {
        return Data.begin();
    }
    const_iterator end() const {
        return Data.end();
    }

    size_t size() const {
        return Size;
    }

    bool empty() const {
        return Size == 0;
    }

    size_t GetBucketIndex(KeyType key) const {
        return HashFunction(key) % Buckets.size();
    }

    size_t GetNextBucketIndex(size_t ind) const {
        ++ind;
        return ind == Buckets.size() ? 0 : ind;
    }

    iterator find(KeyType key) {
        size_t BucketIndex = GetBucketIndex(key);
        size_t CurProbeSequenceLength = 0;

        if (!Buckets[BucketIndex].IsIdle && Buckets[BucketIndex].Iterator->first == key) {
            return Buckets[BucketIndex].Iterator;
        }
        for (; !(Buckets[BucketIndex].IsIdle || Buckets[BucketIndex].ProbeSequenceLength > CurProbeSequenceLength);
               BucketIndex = GetNextBucketIndex(BucketIndex)) {
            if (Buckets[BucketIndex].Iterator->first == key) {
                return Buckets[BucketIndex].Iterator;
            }
            ++CurProbeSequenceLength;
        }
        return end();
    }

    const_iterator find(KeyType key) const {
        size_t BucketIndex = GetBucketIndex(key);
        size_t CurProbeSequenceLength = 0;

        if (!Buckets[BucketIndex].IsIdle && Buckets[BucketIndex].Iterator->first == key) {
            return Buckets[BucketIndex].Iterator;
        }
        for (; !(Buckets[BucketIndex].IsIdle || Buckets[BucketIndex].ProbeSequenceLength > CurProbeSequenceLength);
               BucketIndex = GetNextBucketIndex(BucketIndex)) {
            if (Buckets[BucketIndex].Iterator->first == key) {
                return Buckets[BucketIndex].Iterator;
            }
            ++CurProbeSequenceLength;
        }
        return end();
    }

    ValueType& operator[](const KeyType key) {
        auto it = this->find(key);
        if (it == this->end()) {
            it = this->insert(std::make_pair(key, ValueType()));
        }
        return it->second;
    }

    const ValueType& at(const KeyType key) const {
        const_iterator it = this->find(key);
        if (it == this->end()) {
            throw std::out_of_range("No key found\n");
        } else {
            return it->second;
        }
    }

    void TryRebuild() {
        if (Size * 2 > Buckets.size()) {
            Size = 0;
            Buckets.assign(2 * Buckets.size(), Bucket());

            std::list<std::pair<const KeyType, ValueType>> tmp;
            swap(tmp, Data);

            for (const auto element : tmp) {
                insert(element);
            }
        }
    }

    iterator insert(std::pair<const KeyType, ValueType> element) {
        TryRebuild();

        auto it = find(element.first);
        if (it == end()) {

            size_t CurProbeSequenceLength = 0;
            size_t BucketIndex;

            auto iter = Data.insert(Data.end(), element);
            for (BucketIndex = GetBucketIndex(element.first); !Buckets[BucketIndex].IsIdle;
                 BucketIndex = GetNextBucketIndex(BucketIndex)) {
                if (CurProbeSequenceLength > Buckets[BucketIndex].ProbeSequenceLength) {
                    swap(iter, Buckets[BucketIndex].Iterator);
                    std::swap(CurProbeSequenceLength, Buckets[BucketIndex].ProbeSequenceLength);
                }
                ++CurProbeSequenceLength;
            }

            Buckets[BucketIndex].IsIdle = false;
            Buckets[BucketIndex].Iterator = iter;
            Buckets[BucketIndex].ProbeSequenceLength = CurProbeSequenceLength;

            ++Size;
            it = Buckets[BucketIndex].Iterator;
        }

        return it;
    }

    void erase(KeyType key) {
        auto it = find(key);
        if (it == end()) {
            return;
        }

        size_t BucketIndex = GetBucketIndex(key);
        for (; Buckets[BucketIndex].Iterator->first != key; BucketIndex = GetNextBucketIndex(BucketIndex)) {

        }

        --Size;
        Data.erase(it);

        Buckets[BucketIndex].IsIdle = true;
        while (true) {
            size_t NextBucketIndex = GetNextBucketIndex(BucketIndex);
            if (Buckets[NextBucketIndex].IsIdle || Buckets[NextBucketIndex].ProbeSequenceLength == 0) {
                break;
            }
            --Buckets[NextBucketIndex].ProbeSequenceLength;
            std::swap(Buckets[BucketIndex], Buckets[NextBucketIndex]);
            BucketIndex = NextBucketIndex;
        }
    }

private:
    const size_t DEFAULT_AMOUNT_OF_BUCKETS = 16;
    size_t Size;

    struct Bucket {
        bool IsIdle;
        iterator Iterator;
        size_t ProbeSequenceLength;

        Bucket() {
            IsIdle = true;
            ProbeSequenceLength = 0;
        }
    };

    std::list<std::pair<const KeyType, ValueType>> Data;
    std::vector<Bucket> Buckets;

    Hash HashFunction;
};




const long long MOD = 1000000000000000003;
const int P = 29;


long long sum(long long a, long long b) {
    a += b;
    if (a >= MOD) {
        a -= MOD;
    }
    return a;
}


long long dif(long long a, long long b) {
    a -= b;
    if (a < 0) {
        a += MOD;
    }
    return a;
}


long long mult(long long a, long long b) {
    return ((__int128)(a) * b) % MOD;
}


const int NMAX = 1010;


long long p_pow[NMAX * NMAX];


void fill_p_pow(int n, int m) {
    p_pow[0] = 1;
    for (int i = 1; i < n * m + 10; ++i) {
        p_pow[i] = mult(p_pow[i - 1], P);
    }
}


vector<long long> build_pref_hashes(string& s) {
    int n = s.size();
    vector<long long> hashes(n + 1, 0);

    for (int i = n - 1; i >= 0; --i) {
        hashes[i] = sum(mult(hashes[i + 1], P), s[i]);
    }

    return hashes;
}


char f[NMAX][NMAX];
int n, m;


long long get_hash(int l, int r, vector<long long>& pref_hashes) {
    return dif(pref_hashes[l], mult(pref_hashes[r], p_pow[r - l]));
}


pair<bool, vector<int> > check1(int k, vector<vector<long long> >& pref_hashes) {
    long long hashes[m - k + 1] = {0};

    HashMap<long long, pair<int, int> > acc;
    for (int y = 0; y + k <= m; ++y) {
        long long h = 0;

        for (int i = 0; i < k; ++i) {
            h = sum(mult(h, p_pow[k]), get_hash(y, y + k, pref_hashes[i]));
        }

        if (acc.find(h) != acc.end()) {
            return {true, {acc[h].first, acc[h].second, 0, y}};
        }

        acc[h] = {0, y};
        hashes[y] = h;
    }

    long long h_prv;
    long long h_nxt;
    long long h;

    for (int x = 1; x + k <= n; ++x) {
        for (int y = 0; y + k <= m; ++y) {
            h_prv = get_hash(y, y + k, pref_hashes[x - 1]);
            h_nxt = get_hash(y, y + k, pref_hashes[x + k - 1 - 0]);

            h = sum(mult(dif(hashes[y], mult(h_prv, p_pow[k * (k - 1 - 0)])), p_pow[k]), h_nxt);

            if (acc.find(h) != acc.end()) {
                return {true, {acc[h].first, acc[h].second, x, y}};
            }

            acc[h] = {x, y};
            hashes[y] = h;
        }
    }

    return {false, {-1, -1, -1, -1}};
}


void input() {
    cin >> n >> m;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            cin >> f[i][j];
        }
    }
}


vector<vector<long long> > build_hashes() {
    fill_p_pow(n, m);

    vector<vector<long long> > pref_hashes(n);
    for (int i = 0; i < n; i++) {
        string s = "";
        for (int j = 0; j < m; j++) {
            s += f[i][j];
        }

        pref_hashes[i] = build_pref_hashes(s);
    }

    return pref_hashes;
}

/*
signed main() {
    ios::sync_with_stdio(false);
    cin.tie(0);


    input();
    auto pref_hashes = build_hashes();


    int l_bs = 0, r_bs = min(n, m) + 1, m_bs;
    while (l_bs + 1 < r_bs) {
        m_bs = (l_bs + r_bs) / 2;

        auto it = check1(m_bs, pref_hashes);

        if (it.first) {
            l_bs = m_bs;
        } else {
            r_bs = m_bs;
        }
    }

    auto it = check1(l_bs, pref_hashes);
    cout << l_bs << "\n";

    if (l_bs) {
        cout << it.second[0] + 1 << " " << it.second[1] + 1 << "\n";
        cout << it.second[2] + 1 << " " << it.second[3] + 1 << "\n";
    }

    return 0;
}
 */