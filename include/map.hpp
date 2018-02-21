#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<class Key, class T, class Compare = std::less<Key> > 
class map {
public:
	typedef pair<const Key, T> value_type;
	struct Node;
	class RB_Tree;
	class const_iterator;
	class iterator;
	friend class iterator;
	friend class const_iterator;
	friend class RB_Tree;

private:
	RB_Tree rb_tree;

public:
	static int cmp(const Key &k1, const Key &k2) {
		Compare compare;
		if(compare(k1, k2)) return 1;
		if(compare(k2, k1)) return 0;
		return -1; // equal
	}

#define left ch[0]
#define right ch[1]

	struct Node {
		value_type* value;
		int size;
		bool color; // 1 : red, 0 : black
		Node* ch[2];
		Node* parent;

		Node() : value(nullptr), size(0), color(0), parent(nullptr) { left = right = nullptr; }

		Node(const value_type &_value, int size = 1, bool color = 1,
			Node* lchild = nullptr, Node* rchild = nullptr, Node* pa = nullptr)
			: value(new value_type(_value)), size(size), color(color), parent(pa) {
			left = lchild; right = rchild;
		}

		~Node() { delete value; }
	};

	class RB_Tree {
	private:
		Node* root;
	public:
		Node* null;

		RB_Tree() { root = (null = new Node()); }

		void update(Node* o) {
			o->size = o->left->size + o->right->size + 1;
		}

		bool operator == (const RB_Tree& rhs) const {
			return root == rhs.root;
		}

		RB_Tree & operator = (const RB_Tree& rhs) {
			if (*this == rhs) return *this;
			this->~RB_Tree();
			root = copy(null, rhs.root, rhs.null);
		}

		Node* copy(Node* parent, Node* other, Node* nil) {
			if (other == nil) return null;
			Node* self = new Node(*other->value, other->size, other->color, null, null, parent);
			self->left = copy(self, other->left, nil);
			self->right = copy(self, other->right, nil);
			return self;
		}

		void rotate(Node* o, int d) {
			Node* p = o->ch[d ^ 1];
			o->ch[d ^ 1] = p->ch[d]; 
			p->ch[d]->parent = o;
			p->parent = o->parent;
			if (o->parent == null) root = p;
			else if (o == o->parent->left) o->parent->left = p;
			else o->parent->right = p;
			p->ch[d] = o;
			o->parent = p;
			update(o); update(p);
		}

		Node* kth(int k) const {
			Node* o = root;
			while (o != null && k != rank(o)) {
				if (k < rank(o)) o = o->left;
				else k -= rank(o), o = o->right;
			}
			return o;
		}

		Node* insert(const value_type &value) {
			Node* cur = root;
			Node* parent = null;

			int d;
			while (cur != null) {
				parent = cur;
				d = cmp(cur->value->first, value.first);
				if (d == -1) return cur;
				cur = cur->ch[d];
			}
			Node* new_node = new Node(value, 1, 1, null, null, parent);
			if (parent == null) root = new_node;
			else parent->ch[d] = new_node;
			while (parent != null) {
				update(parent);
				parent = parent->parent;
			}
			insert_fixup(new_node);
			return new_node;
		}

		void insert_fixup(Node* o) {
			while (o->parent->color == 1) {
				bool d = (o->parent == o->parent->parent->right);
				Node *uncle = o->parent->parent->ch[d ^ 1];
				if (uncle->color == 1) {
					o->parent->color = 0;
					uncle->color = 0;
					o->parent->parent->color = 1;
					o = o->parent->parent;
				}
				else {
					if (o == o->parent->right) {
						o = o->parent;
						rotate(o, 0);
					}
					o->parent->color = 0;
					o->parent->parent->color = 1;
					rotate(o->parent->parent, d ^ 1);
				}
			}
			root->color = 0;
		}

		int rank(const Key &key) const {
			int k = 0;
			Node* o = root;
			while (o != null) {
				int d = cmp(o->value->first, key);
				if (d == -1) return k + rank(o);
				if (d == 1) k += rank(o);
				o = o->ch[d];
			}
			return -0x3f3f3f3f;
		}

		int rank(Node* o) const { return o->left->size + 1; }

		Node* find(const Key &key) const {
			Node* o = root;
			while (o != null && cmp(o->value->first, key) != -1) 
				o = o->ch[cmp(o->value->first, key)];
			return o;
		}

		void erase(const Key &key) {
			Node* o = find(key);
			Node* x = null;
			Node* y = null;
			if (o->left == null || o->right == null) y = o; else y = neighbour(o, 1);
			if (y->left != null) x = y->left; else x = y->right;
			x->parent = y->parent;
			if (y->parent == null) root = x;
			else if (y == y->parent->left) y->parent->left = x;
			else y->parent->right = x;
			if (y != o) {
				delete o->value;
				o->value = new value_type(*y->value);
			}
			Node *z = x->parent;
			while (z != null) {
				update(z);
				z = z->parent;
			}
			if (y->color == 0) delete_fixup(x);
			delete y;
		}

		Node* neighbour(Node* o, int d) const {
			if (o->ch[d] != null) {
				o = o->ch[d];
				while (o->ch[d ^ 1] != null) o = o->ch[d ^ 1];
				return o;
			}
			Node* parent = o->parent;
			while (parent != null && o == parent->ch[d]) {
				o = parent;
				parent = parent->parent;
			}
			return parent;
		}

		void delete_fixup(Node* o) {
			while (o != root && o->color == 0) {
				bool d = (o->parent->right == o);
				Node* brother = o->parent->ch[d ^ 1];
				if (brother->color == 1) {
					brother->color = 0;
					o->parent->color = 1;
					rotate(o->parent, d);
					brother = o->parent->ch[d ^ 1];
				}
				if (brother->ch[d]->color == 0 && brother->ch[d ^ 1]->color == 0) {
					brother->color = 1;
					o = brother->parent;
				} else {
					if (brother->ch[d ^ 1]->color == 0) {
						brother->ch[d]->color = 0;
						brother->color = 1;
						rotate(brother, d ^ 1);
						brother = brother->parent; // brother = o->parent->ch[d ^ 1]
					}
					brother->color = brother->parent->color;
					brother->parent->color = 0;
					brother->ch[d ^ 1]->color = 0;
					rotate(brother->parent, d);
					o = root;
				}
			}
			o->color = 0;
		}

		int size() const { return root == null ? 0 : root->size; }

		void clear(Node* o) {
			if (o == null) return;
			clear(o->left);
			clear(o->right);
			delete o;
		}

		~RB_Tree() { clear(root); root = null; }
	};

#undef left
#undef right

	class iterator {
		friend class map;
		friend class const_iterator;

	private:
		Node* pointer;
		const map* belong;

	public:
		iterator(Node* ptr = nullptr, const map* blg = nullptr) : pointer(ptr), belong(blg) {}
		iterator(const iterator &other) : pointer(other.pointer), belong(other.belong) {}

		iterator & operator ++ () {
			if (!pointer || !pointer->value) throw invalid_iterator(); // not belong to this || is end()
			const RB_Tree &rb_tree = this->belong->rb_tree;
			pointer = rb_tree.neighbour(rb_tree.find(pointer->value->first), 1);
			return *this;
		}

		iterator & operator -- () {
			if (!belong) throw invalid_iterator();
			const RB_Tree &rb_tree = this->belong->rb_tree;
			if (pointer && pointer->value) // belong to this && is not end()
				pointer = rb_tree.neighbour(rb_tree.find(pointer->value->first), 0);
			else
				pointer = rb_tree.kth(rb_tree.size());
			if (pointer->value == nullptr) throw invalid_iterator();
			return *this;
		}

		iterator operator ++ (int) {
			iterator ret = *this;
			++(*this);
			return ret;
		}

		iterator operator -- (int) {
			iterator ret = *this;
			--(*this);
			return ret;
		}

		value_type & operator * () const { return *(pointer->value); }
		value_type* operator-> () const noexcept { return pointer->value; }

		bool operator == (const iterator &rhs) const { 
			return belong == rhs.belong && pointer == rhs.pointer; 
		}
		bool operator != (const iterator &rhs) const { return !(*this == rhs); }
		bool operator == (const const_iterator &rhs) const { return *this == rhs.iter; }
		bool operator != (const const_iterator &rhs) const { return !(*this == rhs); }
	};

	class const_iterator {
		friend iterator;
		private:
			iterator iter;
		public:
			const_iterator() {}
			const_iterator(const const_iterator &other) : iter(other.iter) {}
			const_iterator(const iterator &other) : iter(other) {}

			const value_type & operator * () const { return *iter; }
			const value_type* operator -> () const { return &(*iter); }
			bool operator == (const const_iterator &rhs) const { return iter == rhs.iter; }
			bool operator != (const const_iterator &rhs) const { return iter != rhs.iter; }
			const_iterator operator = (const const_iterator &other) { iter = other.iter; return (*this); }
			const_iterator operator ++(int) { return const_iterator(iter++); }
			const_iterator operator ++() { return const_iterator(++iter); }
			const_iterator operator --(int) { return const_iterator(iter--); }
			const_iterator operator --() { return const_iterator(--iter); }
	};

	map() {}
	map(const map &other) { this->rb_tree = other.rb_tree; }

	bool operator == (const map& rhs) const {
		return (cend() == rhs.cend());
	}

	map & operator = (const map &other) {
		if (*this == other) return *this;
		this->rb_tree = other.rb_tree;
		return *this;
	}

	~map() { rb_tree.~RB_Tree(); }

	T & at(const Key &key) {
		value_type* result = rb_tree.find(key)->value;
		if (result) return result->second;
		else throw index_out_of_bound();
	}

	const T & at(const Key &key) const {
		value_type* result = rb_tree.find(key)->value;
		if (result) return result->second;
		else throw index_out_of_bound();
	}

	T & operator [] (const Key &key) {
		return rb_tree.insert(value_type(key, *(new T)))->value->second;
	}

	const T & operator [] (const Key &key) const { return at(key); }

	iterator begin() { return iterator(rb_tree.kth(1), this); }
	const_iterator cbegin() const { return const_iterator(iterator(rb_tree.kth(1), this)); }
	iterator end() { return iterator(rb_tree.null, this); }
	const_iterator cend() const { return const_iterator(iterator(rb_tree.null, this)); }
	
	bool empty() const { return (rb_tree.size() == 0); }
	size_t size() const { return (rb_tree.size()); }
	void clear() { rb_tree.~RB_Tree(); }

	pair<iterator, bool> insert(const value_type &value) {
		int cnt = this->count(value.first);
		return pair<iterator, bool>(rb_tree.insert(value), !cnt);
	}

	void erase(iterator pos) {
		if (pos.belong != this || pos.pointer == nullptr || pos.pointer->value == nullptr) 
			throw invalid_iterator();
		rb_tree.erase(pos.pointer->value->first);
	}

	size_t count(const Key &key) const { return ((rb_tree.rank(key)) > 0 ? 1 : 0); }
	iterator find(const Key &key) { return iterator(rb_tree.find(key), this); }

	const_iterator find(const Key &key) const {
		return const_iterator(iterator(rb_tree.find(key), this));
	}
};

}

#endif