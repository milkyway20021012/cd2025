#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 定義全局變量，用於字符輸入和代碼指針
int in_char;
char current_word[100]; // 緩衝區存儲當前識別符或數字

// 判斷字符是否為字母（用於識別關鍵字/識別符）
int isalpha_custom(int c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// 判斷字符是否為數字（用於字面量token）
int isdigit_custom(int c)
{
    return c >= '0' && c <= '9';
}

// 判斷字符是否為字母數字或下劃線（用於識別符）
int isalnum_custom(int c)
{
    return isalpha_custom(c) || isdigit_custom(c) || c == '_';
}

// 判斷多字節字符是否是全角運算符
int is_fullwidth_operator(unsigned char c1, unsigned char c2, unsigned char c3)
{
    // 檢查這是不是一個UTF-8的多字節字符的開始
    if ((c1 & 0xE0) == 0xE0)
    {
        // 檢查常見的全角字符
        if (c1 == 0xEF && c2 == 0xBC)
        {
            // 全角括號和標點
            if (c3 == 0x88)
                return '('; // （
            if (c3 == 0x89)
                return ')'; // ）
            if (c3 == 0xBB)
                return ';'; // ；
            if (c3 == 0x9A)
                return ':'; // ：
        }
        if (c1 == 0xEF && c2 == 0xBD)
        {
            // 全角大括號
            if (c3 == 0x9B)
                return '{'; // ｛
            if (c3 == 0x9D)
                return '}'; // ｝
        }
        if (c1 == 0xEF && c2 == 0xBC)
        {
            // 全角比較運算符
            if (c3 == 0x9C)
                return '<'; // ＜
            if (c3 == 0x9E)
                return '>'; // ＞
        }
    }
    return 0;
}

// 檢查單詞是否為關鍵字
int is_keyword(char *word)
{
    if (strcmp(word, "int") == 0)
        return 1; // TYPE_TOKEN
    if (strcmp(word, "if") == 0)
        return 2; // IF_TOKEN
    if (strcmp(word, "main") == 0)
        return 3; // MAIN_TOKEN
    if (strcmp(word, "else") == 0)
        return 4; // ELSE_TOKEN
    if (strcmp(word, "while") == 0)
        return 5; // WHILE_TOKEN
    if (strcmp(word, "return") == 0)
        return 1; // 添加 "return" 作為 TYPE_TOKEN
    return 0;     // 不是關鍵字
}

// 獲取識別符或關鍵字
int get_id(FILE *fp)
{
    int i = 0;
    while (isalnum_custom(in_char))
    {
        current_word[i++] = in_char;
        in_char = fgetc(fp); // 從文件獲取下一個字符
    }
    current_word[i] = '\0';

    // 未讀取的字符放回輸入流
    if (in_char != EOF)
        ungetc(in_char, fp);

    int token = is_keyword(current_word);
    if (token != 0)
        return token; // 如果是關鍵字
    return 18;        // ID_TOKEN
}

// 獲取數字（字面量）
int get_number(FILE *fp)
{
    int i = 0;
    while (isdigit_custom(in_char))
    {
        current_word[i++] = in_char;
        in_char = fgetc(fp); // 從文件獲取下一個字符
    }
    current_word[i] = '\0';

    // 未讀取的字符放回輸入流
    if (in_char != EOF)
        ungetc(in_char, fp);

    return 15; // LITERAL_TOKEN
}

// 獲取運算符
int get_operator(FILE *fp)
{
    char op = in_char;

    // 檢查是否是多字節全角字符
    if ((unsigned char)op >= 0xE0)
    {
        unsigned char c1 = op;
        unsigned char c2 = fgetc(fp);
        unsigned char c3 = fgetc(fp);

        // 轉換為對應的半角字符
        int fullwidth_op = is_fullwidth_operator(c1, c2, c3);
        if (fullwidth_op)
        {
            op = fullwidth_op;
        }
        else
        {
            // 如果不是已知的全角字符，將讀取的字符放回
            ungetc(c3, fp);
            ungetc(c2, fp);
        }
    }

    in_char = fgetc(fp); // 獲取下一個字符

    if (op == '=')
    {
        if (in_char == '=')
        {
            in_char = fgetc(fp); // 跳過第二個 '='
            return 11;           // EQUAL_TOKEN
        }
        return 14; // ASSIGN_TOKEN
    }
    if (op == '>')
    {
        if (in_char == '=')
        {
            in_char = fgetc(fp); // 跳過 '='
            return 19;           // GREATER_EQUAL_TOKEN
        }
        return 12; // GREATER_TOKEN
    }
    if (op == '<')
    {
        if (in_char == '=')
        {
            in_char = fgetc(fp); // 跳過 '='
            return 20;           // LESS_EQUAL_TOKEN
        }
        return 13; // LESS_TOKEN
    }
    if (op == '+')
        return 16; // PLUS_TOKEN
    if (op == '-')
        return 17; // MINUS_TOKEN
    if (op == '(')
        return 6; // LEFTPAREN_TOKEN
    if (op == ')')
        return 7; // RIGHTPAREN_TOKEN
    if (op == '{')
        return 8; // LEFTBRACE_TOKEN
    if (op == '}')
        return 9;               // RIGHTBRACE_TOKEN
    if (op == ';' || op == ':') // 處理分號和冒號 (可能是全形分號)
        return 10;              // SEMICOLON_TOKEN

    // 將未識別的字符放回輸入流
    if (in_char != EOF)
        ungetc(in_char, fp);

    return -1; // 不是運算符
}

// 為輸入的內容打印相應的token
void tokenize_code(FILE *fp)
{
    in_char = fgetc(fp); // 從文件獲取第一個字符

    while (in_char != EOF) // 繼續直到到達文件結束
    {
        // 跳過空白字符（空格、換行和製表符）
        if (in_char == ' ' || in_char == '\n' || in_char == '\t' || in_char == '\r')
        {
            in_char = fgetc(fp); // 跳過空白
            continue;
        }

        if (isalpha_custom(in_char)) // 處理識別符和關鍵字
        {
            int token = get_id(fp);
            switch (token)
            {
            case 1:
                printf("%s: TYPE_TOKEN\n", current_word);
                break;
            case 2:
                printf("%s: IF_TOKEN\n", current_word);
                break;
            case 3:
                printf("%s: MAIN_TOKEN\n", current_word);
                break;
            case 4:
                printf("%s: ELSE_TOKEN\n", current_word);
                break;
            case 5:
                printf("%s: WHILE_TOKEN\n", current_word);
                break;
            default:
                printf("%s: ID_TOKEN\n", current_word);
                break;
            }
        }
        else if (isdigit_custom(in_char)) // 處理數字（字面量）
        {
            get_number(fp);
            printf("%s: LITERAL_TOKEN\n", current_word);
        }
        else // 處理運算符
        {
            int op = get_operator(fp);
            if (op != -1)
            {
                switch (op)
                {
                case 11:
                    printf("==: EQUAL_TOKEN\n");
                    break;
                case 14:
                    printf("=: ASSIGN_TOKEN\n");
                    break;
                case 12:
                    printf(">: GREATER_TOKEN\n");
                    break;
                case 13:
                    printf("<: LESS_TOKEN\n");
                    break;
                case 16:
                    printf("+: PLUS_TOKEN\n");
                    break;
                case 17:
                    printf("-: MINUS_TOKEN\n");
                    break;
                case 6:
                    printf("(: LEFTPAREN_TOKEN\n");
                    break;
                case 7:
                    printf("): RIGHTPAREN_TOKEN\n");
                    break;
                case 8:
                    printf("{: LEFTBRACE_TOKEN\n");
                    break;
                case 9:
                    printf("}: RIGHTBRACE_TOKEN\n");
                    break;
                case 10:
                    printf(";: SEMICOLON_TOKEN\n");
                    break;
                case 19:
                    printf(">=: GREATER_EQUAL_TOKEN\n");
                    break;
                case 20:
                    printf("<=: LESS_EQUAL_TOKEN\n");
                    break;
                default:
                    break;
                }
            }
            else
            {
                in_char = fgetc(fp); // 跳過未識別的字符
            }
        }
    }
}

// 從字符串創建內存文件
FILE *create_memory_file(const char *code)
{
    FILE *fp = tmpfile();
    if (fp == NULL)
    {
        return NULL;
    }

    fputs(code, fp);
    rewind(fp);
    return fp;
}

int main()
{
    // 使用您需要分析的特定 main 函數內容
    const char *main_code =
        "int main{\n"
        "int cd2025=5;\n"
        "int cd2025_ = 5；\n"
        "if （cd2025 == 5）｛\n"
        "cd2025_ = 0：\n"
        "｝\n"
        "else {\n"
        "cd2025_ = 1+2+(3+4)+5;\n"
        "}\n"
        "while (cd2025_+cd2025) {\n"
        "cd2025 = cd2025-1;\n"
        "｝\n"
        "return 0;\n"
        "}";

    // 創建內存文件並分析指定的代碼
    FILE *main_file = create_memory_file(main_code);
    if (main_file != NULL)
    {
        tokenize_code(main_file);
        fclose(main_file);
    }
    else
    {
        printf("無法創建臨時文件\n");
        return 1;
    }

    return 0;
}