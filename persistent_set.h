#ifndef PERSISTENT_SET_LIBRARY_H
#define PERSISTENT_SET_LIBRARY_H

#include <cassert>
#include <iterator>
#include <utility>
#include <memory>

template<typename T>
struct persistent_set {
    typedef T value_type;
    struct basic_node;
    struct node;
    struct iterator;
    using const_iterator = iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    persistent_set() noexcept;

    persistent_set(persistent_set const &) noexcept;

    persistent_set &operator=(persistent_set const &other) noexcept = default;

    ~persistent_set() noexcept = default;

    iterator find(T const &value) const;

    std::pair<iterator, bool> insert(T const &value);

    void erase(iterator const &it);

    const_iterator begin() const;

    const_iterator end() const;

    const_reverse_iterator rbegin() const;

    const_reverse_iterator rend() const;

    void swap(persistent_set &other) noexcept;

    bool empty() const noexcept;

    void clear() noexcept;

private:

    void create_fake();

    std::shared_ptr<basic_node> erase_impl(basic_node *pos, basic_node *del);

    std::shared_ptr<basic_node> insert_impl(basic_node *pos, T const &value, basic_node *&result);

    std::shared_ptr<basic_node> fake;
    
    size_t size_;
};

template<typename T>
struct persistent_set<T>::basic_node {
    basic_node();

    basic_node(std::shared_ptr<basic_node> const &left, std::shared_ptr<basic_node> const &right) : left(left),
                                                                                                    right(right) {}

    T &get_value();

    basic_node *min();

    basic_node *max();

    basic_node *next(basic_node *root);

    basic_node *prev(basic_node *root);

    friend struct persistent_set;
    std::shared_ptr<basic_node> left;
    std::shared_ptr<basic_node> right;
};

template<typename T>
struct persistent_set<T>::node : basic_node {
    explicit node(T const &value) : value(value) {}

    node(std::shared_ptr<basic_node> const &left, std::shared_ptr<basic_node> const &right, T const &value)
            : basic_node(left, right), value(value) {}

private:
    friend struct basic_node;
    T value;
};

template<typename T>
struct persistent_set<T>::iterator {
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T const;
    using pointer = T const *;
    using reference = T const &;

    iterator() : node_(nullptr), root(nullptr) {}

    reference operator*() const;

    pointer operator->() const;

    iterator &operator++();

    iterator operator++(int);

    iterator &operator--();

    iterator operator--(int);

    friend bool operator==(iterator const &lhs, iterator const &rhs) {
        return lhs.node_ == rhs.node_;
    }

    friend bool operator!=(iterator const &lhs, iterator const &rhs) {
        return lhs.node_ != rhs.node_;
    }

private:
    friend struct persistent_set;
    basic_node *node_;
    basic_node *root;
    explicit iterator(basic_node *node_, basic_node *root) : node_(node_), root(root) {}
};

template<typename T>
persistent_set<T>::persistent_set() noexcept {
    fake = nullptr;
    size_ = 0;
}

template<typename T>
typename persistent_set<T>::const_iterator persistent_set<T>::begin() const {
    if (!fake || !fake->left) {
        return end();
    }
    return const_iterator(fake.get()->min(), fake.get());
}


template<typename T>
typename persistent_set<T>::const_iterator persistent_set<T>::end() const {
    return const_iterator(fake.get(), fake.get());
}

template<typename T>
typename persistent_set<T>::const_reverse_iterator persistent_set<T>::rbegin() const {
    return const_reverse_iterator(end());
}

template<typename T>
typename persistent_set<T>::const_reverse_iterator persistent_set<T>::rend() const {
    return const_reverse_iterator(begin());
}

template<typename T>
void persistent_set<T>::swap(persistent_set &other) noexcept {
    std::swap(fake, other.fake);
    std::swap(size_, other.size_);
}

template<typename T>
typename persistent_set<T>::iterator persistent_set<T>::find(T const &value) const {
    if (!fake) {
        return end();
    }
    auto current = fake->left.get();
    while (true) {
        if (current == nullptr) {
            return end();
        }
        if (current->get_value() < value) {
            current = current->right.get();
        } else if (value < current->get_value()) {
            current = current->left.get();
        } else {
            return iterator(current, fake.get());
        }
    }
}

template<typename T>
std::pair<typename persistent_set<T>::iterator, bool> persistent_set<T>::insert(T const &value) {
    create_fake();
    auto res = find(value);
    if (res != end()) {
        return {res, false};
    } else {
        basic_node *result = nullptr;
        auto tmp_fake = std::make_shared<persistent_set<T>::basic_node>();
        tmp_fake->left = insert_impl(fake->left.get(), value, result);
        fake = tmp_fake;
        size_++;
        return {persistent_set<T>::iterator(result, fake.get()), true};
    }
}

template<typename T>
void persistent_set<T>::erase(const persistent_set<T>::iterator &it) {
    if (fake && fake->left) {
        auto tmp_fake = std::make_shared<persistent_set<T>::basic_node>();
        tmp_fake->left = erase_impl(fake->left.get(), it.node_);
        fake = tmp_fake;
        size_--;
    }
}

template<typename T>
typename persistent_set<T>::basic_node *persistent_set<T>::basic_node::next(persistent_set::basic_node *root) {
    if (right) {
        return right->min();
    } else {
        auto current = root->left;
        auto result = root;
        while (true) {
            if (current->get_value() < get_value()) {
                current = current->right;
            } else if (get_value() < current->get_value()) {
                result = current.get();
                current = current->left;
            } else {
                return result;
            }
        }
    }
}

template<typename T>
typename persistent_set<T>::basic_node *persistent_set<T>::basic_node::prev(persistent_set::basic_node *root) {
    if (left) {
        return left->max();
    } else {
        auto current = root->left;
        auto result = root;
        while (true) {
            if (current->get_value() < get_value()) {
                result = current.get();
                current = current->right;
            } else if (get_value() < current->get_value()) {
                current = current->left;
            } else {
                return result;
            }
        }
    }
}

template<typename T>
std::shared_ptr<typename persistent_set<T>::basic_node>
persistent_set<T>::insert_impl(persistent_set::basic_node *pos, const T &value, persistent_set::basic_node *&result) {
    if (!pos) {
        auto ret = std::make_shared<typename persistent_set<T>::node>(value);
        result = ret.get();
        return ret;
    } else if (pos->get_value() < value) {
        return std::make_shared<typename persistent_set<T>::node>(pos->left,
                                                                  insert_impl(pos->right.get(), value, result),
                                                                  pos->get_value());
    } else {
        return std::make_shared<typename persistent_set<T>::node>(insert_impl(pos->left.get(), value, result),
                                                                  pos->right, pos->get_value());
    }
}

template<typename T>
std::shared_ptr<typename persistent_set<T>::basic_node>
persistent_set<T>::erase_impl(persistent_set::basic_node *pos, persistent_set::basic_node *del) {
    if (pos == del) {
        if (!del->right) {
            return del->left;
        } else if (!del->left) {
            return del->right;
        } else {
            basic_node *minimum = pos->right->min();
            return std::make_shared<typename persistent_set<T>::node>(pos->left, erase_impl(pos->right.get(), minimum),
                                                                      minimum->get_value());
        }
    } else if (pos->get_value() < del->get_value()) {
        return std::make_shared<typename persistent_set<T>::node>(pos->left, erase_impl(pos->right.get(), del),
                                                                  pos->get_value());
    } else {
        return std::make_shared<typename persistent_set<T>::node>(erase_impl(pos->left.get(), del), pos->right,
                                                                  pos->get_value());
    }
}

template<typename T>
void persistent_set<T>::create_fake() {
    if (!fake)
        fake = std::make_shared<basic_node>();
}

template<typename T>
persistent_set<T>::persistent_set(persistent_set const &other) noexcept {
    fake = other.fake;
    size_ = other.size_;
}

template<typename T>
void persistent_set<T>::clear() noexcept {
    fake = nullptr;
    size_ = 0;
}

template<typename T>
bool persistent_set<T>::empty() const noexcept {
    return size_ == 0;
}

template<typename T>
T &persistent_set<T>::basic_node::get_value() {
    return static_cast<node *>(this)->value;
}

template<typename T>
persistent_set<T>::basic_node::basic_node() {
    left = nullptr;
}

template<typename T>
typename persistent_set<T>::basic_node *persistent_set<T>::basic_node::min() {
    auto current = this;
    while (current->left != nullptr) {
        current = current->left.get();
    }
    return current;
}

template<typename T>
typename persistent_set<T>::basic_node *persistent_set<T>::basic_node::max() {
    auto current = this;
    while (current->right != nullptr) {
        current = current->right.get();
    }
    return current;
}

template<typename T>
typename persistent_set<T>::iterator &persistent_set<T>::iterator::operator++() {
    node_ = node_->next(root);
    return *this;
}

template<typename T>
typename persistent_set<T>::iterator persistent_set<T>::iterator::operator++(int) {
    iterator copy = *this;
    ++*this;
    return copy;
}

template<typename T>
typename persistent_set<T>::iterator &persistent_set<T>::iterator::operator--() {
    node_ = node_->prev(root);
    return *this;
}

template<typename T>
typename persistent_set<T>::iterator persistent_set<T>::iterator::operator--(int) {
    iterator copy = *this;
    --*this;
    return copy;
}

template<typename T>
typename persistent_set<T>::iterator::reference &persistent_set<T>::iterator::operator*() const {
    return node_->get_value();
}

template<typename T>
typename persistent_set<T>::iterator::pointer persistent_set<T>::iterator::operator->() const {
    return &node_->get_value();
}

template<typename T>
void swap(persistent_set<T> &lhs, persistent_set<T> &rhs) {
    lhs.swap(rhs);
}

#endif