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
        Buckets.assign(1, Bucket());
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

    std::size_t GetBucketIndex(KeyType key) const {
        return HashFunction(key) % Buckets.size();
    }

    iterator find(KeyType key) {
        size_t BucketIndex = GetBucketIndex(key);
        if (!Buckets[BucketIndex].IsIdle) {
            auto it = Buckets[BucketIndex].Begin;
            do {
                if (it->first == key) {
                    return it;
                }
            } while(it++ != Buckets[BucketIndex].End);
        }
        return end();
    }

    const_iterator find(KeyType key) const {
        size_t BucketIndex = GetBucketIndex(key);
        if (!Buckets[BucketIndex].IsIdle) {
            auto it = Buckets[BucketIndex].Begin;
            do {
                if (it->first == key) {
                    return it;
                }
            } while(it++ != Buckets[BucketIndex].End);
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
            size_t BucketIndex = GetBucketIndex(element.first);
            if (Buckets[BucketIndex].IsIdle) {
                Buckets[BucketIndex].Begin = Buckets[BucketIndex].End = Data.insert(Data.end(), element);
                Buckets[BucketIndex].Size = 1;
                Buckets[BucketIndex].IsIdle = false;
            } else {
                Buckets[BucketIndex].End = Data.insert(++Buckets[BucketIndex].End, element);
            }
            ++Size;
            it = Buckets[BucketIndex].End;
        }
        return it;
    }

    void erase(KeyType key) {
        auto it = find(key);
        if (it == end()) {
            return;
        }

        size_t BucketIndex = GetBucketIndex(key);
        --Size;
        --Buckets[BucketIndex].Size;
        if (Buckets[BucketIndex].Begin == Buckets[BucketIndex].End) {
            Buckets[BucketIndex].IsIdle = true;
        } else if (Buckets[BucketIndex].Begin->first == key) {
            ++Buckets[BucketIndex].Begin;
        } else if (Buckets[BucketIndex].End->first == key) {
            --Buckets[BucketIndex].End;
        }
        Data.erase(it);
    }

private:
    size_t Size;

    struct Bucket {
        iterator Begin; // inclusive
        iterator End;   // inclusive

        size_t Size;
        bool IsIdle;

        Bucket() {
            Size = 0;
            IsIdle = true;
        }
    };

    std::list<std::pair<const KeyType, ValueType>> Data;
    std::vector<Bucket> Buckets;

    Hash HashFunction;
};
