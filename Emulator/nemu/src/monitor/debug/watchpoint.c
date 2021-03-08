#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

 static WP wp_pool[NR_WP] = {};//wp_pool为WP类型的可容纳32个元素的数组
 static WP *head = NULL, *free_ = NULL;
 static int count_used = 0;

//为什么已经有数组了，还要用链表的结构把它们连起来呢？
void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;//free_指向该数组第一个元素
}

WP* new_wp(void)
{
  if(count_used<24)
  {
    if(count_used == 0)
    {
    head = free_;
    free_ = free_->next;
    head->next = NULL;
    ++count_used;
    return head;
    }
    else
    {
      WP* temp_p = head;
      while(temp_p->next != NULL)
        temp_p = temp_p->next;
      temp_p->next = free_;
      free_= free_->next;
      temp_p->next->next = NULL;
      ++count_used;
      return temp_p->next;
    }
  }
  else
  {
    printf("All watchpoints have been used!\n");
    assert(0);
  }
}

void free_wp(WP *wp)//暗示至少使用了一个节点
{
  WP* temp_p1 = head;
  int node_id = wp->NO;
  WP* temp_p2 = free_;
  if(temp_p1->NO == node_id)
  {
    head = temp_p1->next;
    temp_p1->next = NULL;
  }
  else
  { 
    while(temp_p1->next->NO != node_id)
    {
      temp_p1 = temp_p1->next;
    }
    temp_p1->next = wp->next;
    wp->next = NULL;
  }
  if(temp_p2->next ==NULL)
  {
    if(temp_p2->NO > node_id)
    {
      wp->next = temp_p2;
      free_ = wp;
    }
    else
    {
      temp_p2->next = wp;
      wp->next = NULL;
    }
  }
  else
  {
    while(temp_p2->next!=NULL && temp_p2->next->NO < node_id)
    {
      temp_p2 = temp_p2->next;
    }
    wp->next = temp_p2->next;
    temp_p2->next = wp;
  } 
  --count_used;
}

void delete_w (int num)
{
 WP* temp_p = head;
  while(temp_p!=NULL && temp_p->NO != num)
    temp_p = temp_p->next;
  if(temp_p == NULL)
  {
    printf("No watchpoint named after %d is used!\n", num);
  }  
  else
  {
    free_wp(temp_p);
  }  
}
bool update_all_w(void)
{
  WP* temp_p3 = head;
  bool judge = true;
  bool* p_judge = &judge;
  bool whether = false;
  while(temp_p3 != NULL)
  {
    temp_p3->former_value = temp_p3->latter_value;
    temp_p3->latter_value = expr(temp_p3->content, p_judge);
    if(*p_judge)
    {
      if(temp_p3->former_value != temp_p3 -> latter_value)
        {
          whether = true;
          printf("The value of watchpoint %d changed!\n", temp_p3->NO);
        }

    }
    else
    {
      printf("Something wrong in the valuation of expr!\n");
      assert(0);
    }
    temp_p3 = temp_p3->next;
  }
  return whether;
}
void print_all_watchpoints(void)
{
  WP* temp_p4 = head;
  if(head==NULL)
    printf("There exists no watchpoint now!\n");
  while(temp_p4 != NULL)

  {
    printf("Num   Expression   Value\n");
    printf("%d    %s           %d\n", temp_p4->NO, temp_p4->content, temp_p4->latter_value);
    temp_p4 = temp_p4->next;
  } 
}



/* TODO: Implement the functionality of watchpoint */


