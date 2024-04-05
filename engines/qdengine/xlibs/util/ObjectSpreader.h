#include <stack>
#include <vector>
#include "Handle.h"

#include "Serialization\Range.h"

template<class T>
class CyclicList {
private:
	struct Node : public ShareHandleBase{
		T data;
		ShareHandle<Node> prev, next;
	};
public:
	class iterator {
		friend class CyclicList;
	public:
		iterator& operator++() {
			xassert(node);
			node = node->next;
			return *this;
		}
		iterator& operator--() {
			xassert(node);
			node = node->prev;
			return *this;
		}
		iterator operator+(int offset) {
			iterator it (node);
			for (int i = 0; i < offset; ++i)
				++it;
			return it;
		}
		iterator operator-(int offset) {
			iterator it (node);
			for (int i = 0; i < offset; ++i)
				--it;
			return it;
		}
		T& operator* () {
			xassert(node);
			return node->data;
		}
		bool operator== (const iterator& rhs) {
			return node == rhs.node;
		}
		bool operator!= (const iterator& rhs) {
			return node != rhs.node;
		}
	private:
		iterator(Node* _node) {
			node = _node;
		}
		ShareHandle<Node> node;
	};

	CyclicList() {
		begin_ = 0;
	}
	~CyclicList() {
		if(begin_) {
			begin_->prev->next = 0;
			Node* node = begin_;
			do {
				node->prev = 0;
				node = node->next;
			} while(node);
			begin_ = 0;
		}
	}

	iterator begin() {
		return iterator(begin_);
	}

	void insert (iterator it, const T& data) {
		ShareHandle<Node> new_node = new Node;
		new_node->prev = it.node;
		new_node->next = it.node->next;
		new_node->next->prev = new_node;
		it.node->next = new_node;
		new_node->data = data;
	}

	void insert (const T& data) {
		//ShareHandle<
		if (begin_ == 0) {
			begin_ = new Node;
			begin_->data = data;
			begin_->next = begin_;
			begin_->prev = begin_;
		} else {
			insert (begin(), data);
		}
	}

	void erase (iterator it) {
		xassert (it.node);
		ShareHandle<Node> next = it.node->next;
		ShareHandle<Node> prev = it.node->prev;
		next->prev = prev;
		prev->next = next;
		xassert (next && prev);
		it.node->next = 0;
		it.node->prev = 0;
		if (it.node == begin_) {
			begin_ = prev;
		}
	}

	void clear ()
	{
		iterator current = begin();
		if (current.node) {
			do {
				iterator next = current + 1;
				current.node->next = 0;
				current.node->prev = 0;
				current.node = next.node;
			} while (current.node != begin_);
			current.node = 0;
			begin_ = 0;
		}
	}

	int size() {
		if (begin_) {
			iterator first = begin();
			iterator it = first;
			int count = 0;
			do {
				++count;
				//xassert (current->next->prev == current);
				//xassert (current->prev->next == current);
				++it;
			} while (it != first);
			return count;
		} else {
			return 0;
		}
	}

	ShareHandle<Node> begin_;
};

class ObjectSpreader {
public:
	struct Node : public ShareHandleBase {
		Node () : index (0)
		{}

		ShareHandle<Node> next;
		ShareHandle<Node> prev;
		int index;
	};

    struct Circle {
		Circle()
		: active (true)
		{}
        inline bool intersect (const Circle& rhs) const {
            return (rhs.position - position).norm2 () < (radius + rhs.radius) * (radius + rhs.radius);
        }
        Vect2f position;
        float radius;
		bool active;
    };


    ObjectSpreader ();
    void setRadius (const Rangef& _radius) { radius_ = _radius; }
	void setSeed (int seed) { random_.set (seed); }

	void clear ();

	typedef CyclicList<int> Outline;

	Circle& getCircle(int index) {
		xassert(index >= 0 && index < objects_.size ());
		return objects_[index];
	}
	
	Outline::iterator getOutline () { return outline_.begin(); }
	int outlineLength () { return outline_.size(); }

    typedef std::vector<Circle> CirclesList;
	const CirclesList& circles () { return objects_; }

	const Circle& addCircle (float radius);

	template<class Pred> void fill (Pred placementChecker) {
		clear ();
		newRadius ();
		newRadius ();

		Circle c;
		c.position.set (0.0f, 0.0f);
		c.radius = newRadius ();
		objects_.push_back (c);
		
		outline_.insert (0);

		if (!placementChecker (c))
			return;

		for(;;) {
			int count = outlineLength ();
			int offset = getNextCircle ();

			int lastOne = addCircle (newRadius(), outline_.begin() + offset);
			getCircle(lastOne).active = placementChecker (getCircle (lastOne));

			Outline::iterator current = outline_.begin();
			bool inactive = true;
			do {
				if (getCircle (*current).active) {
					inactive = false;
					break;
				}
				++current;
			} while (current != outline_.begin());
			if (inactive || objects_.size() > 1500) {
				break;
			}
		}
	}

private:
	int addCircle (float radius, Outline::iterator node);
	int addCircle (float radius, int outlineIndex);
	void eraseInactive ();

	// вычисления
	float angle (Outline::iterator node1, Outline::iterator node2, Outline::iterator node3);
    Circle thirdCircle (const Circle& c1, const Circle& c2, float radius);
    Circle adjacentCircle (const Circle& circle, float radius);

	int getNextCircle ();
    void repairOutline ();

	void eraseNode (Outline::iterator);
	float newRadius () {
		float result = nextRadius_;
		nextRadius_ = random_.fabsRnd (radius_.length()) + radius_.minimum();
        return result;
    }
	void setNextRadius (float radius) {
		nextRadius_ = radius;
	}
	float nextRadius () {
		return nextRadius_;
	}
private:
	RandomGenerator random_;
    Rangef radius_;
    CirclesList objects_;
	Outline outline_;
	float nextRadius_;
};
