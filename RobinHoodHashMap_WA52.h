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
//
    void TryRebuild() {
        if (Size * 9 + 18 > Buckets.size()) {
            Size = 0;
            Buckets.assign(9 * Buckets.size() + 18, Bucket());

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
            it = iter;
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
        }

        return it;
    }

    void erase(KeyType key) {
        auto it = find(key);
        if (it == end()) {
            return;
        }

        size_t BucketIndex;
        for (BucketIndex = GetBucketIndex(key); Buckets[BucketIndex].Iterator->first != key;
             BucketIndex = GetNextBucketIndex(BucketIndex)) {

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
