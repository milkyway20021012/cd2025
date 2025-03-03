#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define version1 0
#define version2 1
// 版本一 搜尋字元採用逐個搜尋
#if version1
// 定義鏈表結構
typedef char LListDatatype;

typedef struct LinkList
{
    LListDatatype data;    // 存儲字元
    int total;             // 計算該字元的個數
    struct LinkList *next; // 指向下一個節點
} LList;

// 創建節點
LList *CreateNode(LListDatatype x)
{
    LList *node = (LList *)malloc(sizeof(LList));
    if (node == NULL)
    {
        perror("malloc fail");
        return NULL;
    }
    node->data = x;
    node->total = 1; // 初始計數為 1
    node->next = NULL;
    return node;
}

// 插入新的節點或者更新total的數量
void LListInsertOrUpdate(LList **pphead, LListDatatype x)
{
    LList *cur = *pphead;
    while (cur)
    {
        if (cur->data == x)
        {
            cur->total++;
            return;
        }
        cur = cur->next;
    }

    // 如果字元不在鏈表中，則插入新節點
    LList *newnode = CreateNode(x);

    if (*pphead == NULL)
    {
        *pphead = newnode;
    }
    else
    {
        LList *tail = *pphead;
        while (tail->next != NULL)
        {
            tail = tail->next;
        }
        tail->next = newnode;
    }
}

// 輸出
void Print(LList *phead)
{
    if (phead == NULL)
    {
        return;
    }
    LList *cur = phead;
    while (cur)
    {
        printf("%c : %d\n", cur->data, cur->total);
        cur = cur->next;
    }
}
// 銷毀
void LListDestroy(LList **pphead)
{
    LList *cur = *pphead;
    while (cur)
    {
        LList *tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    *pphead = NULL;
}

int main()
{
    // 讀取自身程式碼文件
    FILE *fp = fopen("main.c", "r");
    if (fp == NULL)
    {
        perror("fopen fail");
        return 1;
    }

    LList *plist = NULL;
    int ch;

    while ((ch = fgetc(fp)) != EOF)
    {
        if (ch < 32 || ch > 126) // 過濾掉無法正常顯示的字元
        {
            continue;
        }
        LListInsertOrUpdate(&plist, ch);
    }

    fclose(fp); // 關閉文件

    Print(plist); // 打印Link-List
    LListDestroy(&plist);

    return 0;
}

#endif

// 版本二 搜尋優化 -- 採用的是Hash table來完成
/*
為什麼可以利用 Hash table 來完成？ 先說結論 使用Hash table 來進行搜尋可以將時間複雜度降到O(1) 相較逐個搜尋的O(n)有著非常顯著的提升

原理大概是這樣：使用哈希將字元轉換為索引

例如，hashTable[ch] 直接對應字元 ch 在鏈表中的節點指標。
查找 ch 是否存在時，不用遍歷鏈表，只需要查詢 hashTable[ch] 是否為 NULL
從上可以看出我們可以不用逐個節點進行搜尋
*/
#if version2
// 定義鏈表結構
typedef char LListDatatype;

typedef struct LinkList
{
    LListDatatype data;    // 存儲字元
    int total;             // 計算該字元的個數
    struct LinkList *next; // 指向下一個節點
} LList;

// 創建節點
LList *CreateNode(LListDatatype x)
{
    LList *node = (LList *)malloc(sizeof(LList));
    if (node == NULL)
    {
        perror("malloc fail");
        return NULL;
    }
    node->data = x;
    node->total = 1; // 初始計數為 1
    node->next = NULL;
    return node;
}
// 定義哈希表（256是為了搭配ASCII)
LList *hashTable[256] = {NULL};
// 插入新的節點或者更新total的數量
void LListInsertOrUpdate(LList **pphead, char x)
{
    if (hashTable[(unsigned char)x] != NULL) // O(1)的查找時間複雜度
    {
        hashTable[(unsigned char)x]->total++; // 當前字元不為空 總數就+1
        return;
    }

    // 如果字元不在鏈表中，插入新節點
    LList *newnode = CreateNode(x);
    if (*pphead == NULL)
    {
        *pphead = newnode;
    }
    else
    {
        LList *tail = *pphead;
        while (tail->next != NULL)
        {
            tail = tail->next;
        }
        tail->next = newnode;
    }

    // 更新哈希表，讓 hashTable[x] 指向新節點
    hashTable[(unsigned char)x] = newnode;
}

// 輸出
void Print(LList *phead)
{
    if (phead == NULL)
    {
        return;
    }
    LList *cur = phead;
    while (cur)
    {
        printf("%c : %d\n", cur->data, cur->total);
        cur = cur->next;
    }
}

void LListDestroy(LList **pphead)
{
    LList *cur = *pphead;
    while (cur)
    {
        LList *tmp = cur;
        hashTable[(unsigned char)cur->data] = NULL; // 清除哈希表中的對應記錄
        cur = cur->next;
        free(tmp);
    }
    *pphead = NULL;
}

int main()
{
    // 讀取自身程式碼文件
    FILE *fp = fopen("main.c", "r");
    if (fp == NULL)
    {
        perror("fopen fail");
        return 1;
    }

    LList *plist = NULL;
    int ch;

    while ((ch = fgetc(fp)) != EOF)
    {
        if (ch < 32 || ch > 126) // 過濾掉無法正常顯示的字元
        {
            continue;
        }
        LListInsertOrUpdate(&plist, ch);
    }

    fclose(fp); // 關閉文件

    Print(plist); // 打印Link-List
    LListDestroy(&plist);

    return 0;
}
#endif