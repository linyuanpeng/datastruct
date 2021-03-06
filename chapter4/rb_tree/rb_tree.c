#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include "rb_tree.h"

/* the NULL node of tree */
#define _NULL(rbtree) (&((rbtree)->null))
/*每个结点要么是红的要么是黑的。  
 *根结点是黑的。  
 *每个叶结点（叶结点即指树尾端NIL指针或NULL结点）都是黑的。  
 *如果一个结点是红的，那么它的两个儿子都是黑的。  
 *对于任意结点而言，其到叶结点树尾端NIL指针的每条路径都包含相同数目的黑结点。 
 *
 */

static int verif_rb_node(c_rb_tree_t *tree,c_rb_node_t *node)
{
	
#ifdef RB_TREE_DEBUG
	printf("RB_TREE_DEBUG\n");
	int black_num_left = 0;
	int black_num_right = 0;
	if(node != NULL)
	{
		if(node->color == RED)
		{
			assert(node != tree->root);
			assert(node->parent != NULL);
			assert(node->parent->color == BLACK);	
			if(node->left != NULL)
				assert(node->left->color == BLACK);
			if(node->right != NULL)
				assert(node->right->color == BLACK);
		}else
		{
			if(node->left != NULL)
				assert(tree->compare(node->key,node->left->key) > 0);

			if(node->right != NULL)
				assert(tree->compare(node->key,node->right->key) < 0);

		}
		black_num_left = verif_rb_node(tree,node->left);
		black_num_right = verif_rb_node(tree,node->right);
		assert(black_num_left == black_num_right);
		return node->color == BLACK ? black_num_left + 1 : black_num_left;
	}
	return 1;
#else
	return 0;
#endif	
}

static void  verif_rb_tree(c_rb_tree_t *tree)
{
	if(tree == NULL || tree->size == 0)
		return;
	verif_rb_node(tree,tree->root);
}

void static rbnode_set_red(c_rb_node_t *rbnode)
{
	if(rbnode == NULL)
		return;
	rbnode->color = RED;
}
void static rbnode_set_black(c_rb_node_t *rbnode)
{
	if(rbnode == NULL)
		return;
	rbnode->color = BLACK;
}
static  int rbnode_is_red(c_rb_node_t *rbnode)
{
	return rbnode == NULL ? 0:(rbnode->color == RED);
}
static  int rbnode_is_black(c_rb_node_t *rbnode)
{
	return rbnode == NULL ? 1:(rbnode->color == BLACK);
}


/*         (x)                            (y)
 *        /   \                          /   \
 *    (y)     (c)											 (a)    (x) 
 *   /    \                                  /    \
 * (a)    (b)																 b     c
 *  右旋
 *
 */
c_rb_node_t* c_rb_subtree_min(c_rb_node_t *node)
{
	if(node == NULL)
		return NULL;

	if(node->left != NULL)
		node = node->left;

	return node;
}

c_rb_node_t* c_rb_subtree_max(c_rb_node_t *node)
{
	if(node == NULL)
		return NULL;

	if(node->right != NULL)
		node = node->right;
	return node;
}
static void free_node(c_rb_node_t *node)
{
	if(node == NULL)
		return ;

	if(node->left != NULL)
		free_node(node->left);
	else
		free_node(node->right);
	free(node);
}
static c_rb_node_t* search_node(c_rb_node_t *node, 
		const void *key,int(*compare)(const void *,const void *))
{
	c_rb_node_t *result = NULL;
	int comp = 0;

	assert(compare != NULL);

	if( node == NULL)
		return NULL;

	comp = compare(key,node->key);
	if( comp  < 0) // 要搜索的node是该节点的左子树
		result = search_node(node->left,key,compare);
	else if (comp > 0)
		result = search_node(node->right,key,compare);
	else
		result = node;

	return result;
}

static c_rb_node_t *search_tree(c_rb_tree_t *rbtree, const void *key)
{

	if( rbtree == NULL )
		return NULL;

	return search_node(rbtree->root,key,rbtree->compare);
}
/*         (x)                            (y)
 *        /   \                          /   \
 *    (3y)     (1c)                   (2a)    (2x) 
 *   /    \                                  /    \
 * (2a)    (1b)                            1b     1c
 *  右旋
 *
 */
static c_rb_node_t* rotate_right(c_rb_tree_t *rb_tree, c_rb_node_t *node_x)
{
	c_rb_node_t *node_y = NULL;
	c_rb_node_t *node_b = NULL;
	c_rb_node_t *node_parent = NULL;

	if(node_x == NULL || rb_tree == NULL || node_x->left == NULL)
		printf("rotate_right error \n");

	assert(node_x != NULL);
	assert(rb_tree != NULL);
	assert(node_x->left !=  NULL);

	node_y = node_x->left;
	node_b = node_y->right;
	node_parent = node_x->parent;

	node_y->right = node_x;
	node_y->parent = node_parent;

	node_x->left = node_b;
	node_x->parent = node_y;

	if(node_b != NULL)
		node_b->parent = node_x;

	assert((node_parent == NULL) || (node_parent->left == node_x) || (node_parent->right) == node_x);
	if( node_parent == NULL)
		rb_tree->root = node_y;
	else
	{
		if(node_parent->left == node_x)
			node_parent->left = node_y;
		else 
			node_parent->right = node_y;
	}

	return node_y;
}
/*
 *    (x)                        (y)             
 *   /   \                      /   \
 *(1a)    (3y)              (2x)     (2c)
 *        /   \            /    \
 *       (1b)  (2c)      1a     1b 
 */
static c_rb_node_t* rotate_left(c_rb_tree_t *rb_tree, c_rb_node_t *node_x)
{ 
	c_rb_node_t *node_y;
	c_rb_node_t *node_parent;
	c_rb_node_t *node_b;

	if(node_x == NULL || rb_tree == NULL || node_x->right == NULL)
		printf("rotate_left error \n");

	assert(node_x != NULL);
	assert(rb_tree != NULL);
	assert(node_x->right !=  NULL);

	node_y = node_x->right;
	node_parent = node_x->parent;
	node_b = node_y->left;

	node_y->left = node_x;
	node_y->parent = node_parent;

	node_x->parent = node_y;
	node_x->right = node_b;

	if(node_b != NULL)
		node_b->parent = node_x;

	assert((node_parent == NULL) || (node_parent->left == node_x) || (node_parent->right) == node_x);

	if( node_parent == NULL)
		rb_tree->root = node_y;
	else
	{
		if(node_parent->left == node_x)
			node_parent->left = node_y;
		else 
			node_parent->right = node_y;
	}

	return node_y;
}
/*node=>parent->parent is left node
 *
 * case1 
 *   A->parent(B) and A->uncle is red
 *   set B black
 *   set B black
 *   set D red
 *   set newA = D
 *        (+) D             (-)
 *       /    \           /     \
 *  B (-)     (-) C    B(+)     (+) C
 *    /  \    /  \      /\       / \
 * A(-)   *  *    *  A(-) *     *   *
 *
 * case2
 *   A->parent(B) is red and A->uncle(B) is black,and A is right
 *   set newA = A->parent(B)
 *   rotate_left(newA)
 *   
 *        (+) D             
 *       /    \
 *  B (-)     (+) C    
 *    /  \    /  \
 *  *   A(-) *    *  
 *
 *
 *        (+) D             
 *       /    \
 *   A (-)   (+) C    
 *    /      /  \
 *  B(-)    *    *  
 *
 *
 * case3
 *   A->parent(B) is red and A->uncle(B) is black,and A is left 
 *	 set B black
 *	 set D red
 *   rotate_right(D)
 *
 *        (+) D             
 *       /    \
 *   B (-)   (+) C    
 *    /  \   /  \
 *  A(-)  *  *   *  
 *
 *
 *
 *
 *
 */
static void insert_fixup(c_rb_tree_t *tree, c_rb_node_t *node)
{
	c_rb_node_t *parent;
	c_rb_node_t *uncle;

	while(rbnode_is_red(node->parent)){
		parent = node->parent;
		if(parent == parent->parent->left){
			uncle = parent->parent->right;

			if(rbnode_is_red(uncle)){  /* case 1 p , u is rea*/
				rbnode_set_black(parent);
				rbnode_set_black(uncle);
				rbnode_set_red(parent->parent);
				node = parent->parent;
			}else{
				if(node == parent->right) {/* case 2 p is read u:black,node is right child */
					node = parent;
					rotate_left(tree,node);
					parent = node->parent;
				}
				/* case 3  p red u black node is left */
				rbnode_set_black(parent);
				rbnode_set_red(parent->parent);
				rotate_right(tree,parent->parent);
			}
		}else{/* parent is the right child */
			uncle = parent->parent->left;
			if(rbnode_is_red(uncle)){
				rbnode_set_black(parent);
				rbnode_set_red(parent->parent);
				node = parent->parent;
			}else{
				if(node == parent->left){
					node = parent;
					rotate_right(tree,node);
					parent = node->parent;
				}
				rbnode_set_black(parent);
				rbnode_set_red(parent->parent);
				rotate_left(tree,parent->parent);
			}
		}
	}

	/* set root to black */
	rbnode_set_black(tree->root);
}

c_rb_tree_t *c_rbtree_create(int(*compare)(const void *,const void *))
{
	c_rb_tree_t *tree  = NULL;

	tree  = (c_rb_tree_t *)malloc(sizeof(c_rb_tree_t));
	if(tree == NULL)
	{
		printf("malloc new tree failed \n");
		return NULL;
	}
	tree->root = NULL;
	tree->size = 0;
	tree->compare = compare;

	return tree;
}
void c_rbtree_destroy(c_rb_tree_t *rbtree)
{
	if(rbtree == NULL)
		return;
	free_node(rbtree->root);
}
int c_rbtree_insert(c_rb_tree_t *tree,void *key,void *value)
{
	c_rb_node_t *new = NULL;
	c_rb_node_t *index = NULL;
	int comp = 0;

	if(tree == NULL || key == NULL)
		return -1;

	new = (c_rb_node_t *)malloc(sizeof(c_rb_node_t));
	if(new == NULL){
		printf("malloc new node failed \n");
		return -1;
	}

	new->color = RED;
	new->key = key;
	new->value = value;
	tree->size++;

	if(tree->root == NULL){/* rbtree is empty */
		new->color = BLACK;
		tree->root = new;
		new->parent = NULL;
	}else{
		index = tree->root;
		while(1){
			comp = tree->compare(index->key,new->key);
			if(comp == 0){
				free(new);
				return 0;
			}else if( comp < 0 ){/* index < new*/
				if(index->right == NULL){
					index->right = new;
					new->parent = index;
					break;
				}else{
					index = index->right;
				}
			}else{
				if(index->left == NULL){
					index->left = new;
					new->parent = index;
					break;
				}else{
					index = index->left;
				}
			}	
		}
	}
	new->left = NULL;
	new->right = NULL;
	if(rbnode_is_red(new->parent))
		insert_fixup(tree,new);

	verif_rb_tree(tree);
	return 0;
}
/*case 1
 *   A brother is red
 *   set B red,set D black,left B
 *   set new Brohter = C
 *                                  
 *        (+) B                         (+) D
 *      /     \                       /     \
 *  A (+)     (-) D               B (-)      (+) E 
 *	  / \     /  \                 /   \
 *         C (+) (+) E        A  (+)    (+) C
 *
 *
 *case 2
 *   A brother D is black,
 *   and D left child and right child is black
 *   set A parent(B) red,send new parent = B 
 *                                  
 *        (+-) B                       (-) B
 *      /     \                       /     \
 *  A (+)     (+) D               A (+)      (+) D 
 *	  / \     /  \                          /   \
 *         C (+) (+) E                  C (+)    (+) E
 *
 *
 *case 3
 *   A brother D is black,
 *   and D left child is red and right child is black
 *   set left 
 *                                  
 *        (+-) B                        (+-) B
 *      /     \                       /     \
 *  A (+)     (+) D               A (+)      (+) C 
 *	  / \     /  \                          /   \
 *        C (-)  (+) E                          (-) D
 *                                              /  \
 *                                                  (+) E
 *
 *case 4
 *   A brother D is black,
 *   and D left child is red and right child is black
 *   set left 
 *                                  
 *        (+-) B                        (+-) D
 *      /     \                       /     \
 *  A (+)     (+) D               B (+)      (-) E
 *	  / \     /  \                  /  \
 *         c(+-) (-) E          A (+)  C(+-)          
 *                                              
 *                                            
 */
void  remove_fixup(c_rb_tree_t *rbtree,c_rb_node_t *node)
{
	c_rb_node_t *brother = NULL;

	while((node != rbtree->root) && rbnode_is_black(node))
	{
		if(node == node->parent->left)
		{
			brother = node->parent->right;
			if(rbnode_is_red(brother))/* brother is red*/
			{
				rbnode_set_black(brother);
				rbnode_set_red(node->parent);
				rotate_left(rbtree,node);
				brother = node->parent->right;
			}

			if(rbnode_is_black(brother->right) && rbnode_is_black(brother->left))
			{
				rbnode_set_red(brother);
				node = node->parent;
			}else
			{
				if(rbnode_is_black(brother->right))
				{
					rbnode_set_black(brother->left);
					rbnode_set_red(brother);
					rotate_right(rbtree,brother);
					brother = node->parent->right;
				}

				brother->color = node->parent->color;
				rbnode_set_black(node->parent);
				rbnode_set_black(brother->right);
				rotate_left(rbtree,node->parent);
				node = rbtree->root;/* break loop*/
			}
		}else/* node is right child*/
		{
			brother = node->parent->left;

			if(rbnode_is_red(brother))/*case1: brother is red*/
			{
				rbnode_set_black(brother);
				rbnode_set_red(node->parent);
				rotate_right(rbtree,node);
				brother = node->parent->left;
			}

			/* case 2 b is black and b->left is black and b->right is black*/
			if(rbnode_is_black(brother->right) && rbnode_is_black(brother->left))
			{
				rbnode_set_red(brother);
				node = node->parent;
			}else
			{
				/* case 3 is black and b->left is black and b->right is red*/
				if(rbnode_is_black(brother->left))
				{
					rbnode_set_black(brother->right);
					rbnode_set_red(brother);
					rotate_left(rbtree,brother);
					brother = node->parent->left;
				}

				brother->color = node->parent->color;
				rbnode_set_black(node->parent);
				rbnode_is_black(brother->left);
				rotate_right(rbtree,node->parent);
				node = rbtree->root;/* break loop*/
			}
		}
	}
	rbnode_is_black(node);
}
static void transplant(c_rb_tree_t *rbtree,c_rb_node_t *node1,c_rb_node_t *node2)
{
	if(node1->parent == NULL)
		rbtree->root = node2;
	else if(node1->parent->left == node1)
		node1->parent->left = node2;
	else
		node1->parent->right = node2;

	if(node2 != NULL)
		node2->parent = node1->parent;
}

static void _remove(c_rb_tree_t *rbtree,c_rb_node_t *node)
{
	color_t color;
	c_rb_node_t *x;

	if((rbtree == NULL) || (node) == NULL) 
		return;


	if((node->left != NULL) && (node->right != NULL))
	{
		c_rb_node_t *min = c_rb_subtree_min(node->right);
		node->key = min->key;
		node->value = min->value;
		node = min;
	}

	if((node->left == NULL) && (node->right == NULL))
	{
		if(node == rbtree->root)
		{
			free(node);
			rbtree->size--;
#ifdef DEBUG
			assert(rbtree->size == 0);
#endif
			return;
		}
		x = node->parent;
	}else if(node->left == NULL)
	{
		x = node->right;
		transplant(rbtree,node,node->right);
	}else
	{
		x = node->left;
		transplant(rbtree,node,node->left);
	}

	color = rbnode_is_black(node);
	free(node);
	rbtree->size--;

	if(color == BLACK)
		remove_fixup(rbtree,x);

	verif_rb_tree(rbtree);

}
void c_rbtree_remove1(c_rb_tree_t *rbtree,void const *key,void **rkey,void **rvalue)
{
	c_rb_node_t *node;

	assert(rbtree != NULL);
	node = search_tree(rbtree,key);
	if(node == NULL)
		return;

	if(rkey != NULL)
		*rkey = node->key;
	if(rvalue != NULL)
		*rvalue = node->value;

	 _remove(rbtree,node);
}

void c_rbtree_remove2(c_rb_tree_t *rbtree,c_rb_node_t *node)
{
	_remove(rbtree,node);
}



