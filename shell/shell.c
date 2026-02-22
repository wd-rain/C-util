#include "shell.h"

#define assertAction(exp, action) if(!(exp)) { action; }

bool shellCheckPermission(shell_t *shell, shell_cmd_t *command)
{
    return (shell->permission == 0) || (shell->permission >= command->attr.attrs.permission);
}

static void shellWriteByte(shell_t *shell, uint8_t data)
{
    shell->write(&data, 1);
}

void shellInsertByte(shell_t *shell, uint8_t data)
{
    // 判断输入数据是否过长
    if (shell->parser.length >= shell->parser.bufferSize - 1)
    {
        // 提示
        return;
    }

    if (shell->parser.cursor == shell->parser.length)
    {
        shell->parser.buffer[shell->parser.length++] = data;
        shell->parser.buffer[shell->parser.length] = 0;
        shell->parser.cursor++;
        shellWriteByte(shell, shell->status.isChecked ? data : '*');
    }
    else if (shell->parser.cursor < shell->parser.length)
    {
        // 插入字符
        for (uint32_t i = shell->parser.length - shell->parser.cursor; i > 0; i--)
        {
            shell->parser.buffer[shell->parser.cursor + i] = shell->parser.buffer[shell->parser.cursor + i - 1];
        }
        shell->parser.buffer[shell->parser.cursor++] = data;
        shell->parser.buffer[++shell->parser.length] = 0;
        for (short i = shell->parser.cursor - 1; i < shell->parser.length; i++)
        {
            shellWriteByte(shell, shell->status.isChecked ? shell->parser.buffer[i] : '*');
        }
        for (short i = shell->parser.length - shell->parser.cursor; i > 0; i--)
        {
            shellWriteByte(shell, '\b');
        }
    }
}

/**
 * @brief shell 写字符串
 *
 * @param shell shell对象
 * @param string 字符串数据
 *
 * @return unsigned short 写入字符的数量
 */
unsigned short shellWriteString(shell_t *shell, const char *string)
{
    unsigned short count = 0;
    const char *p = string;
    assertAction(shell->write, return 0);
    while (*p++)
    {
        count++;
    }
    return shell->write((const uint8_t *)string, count);
}

/**
 * @brief shell删除命令行数据
 *
 * @param shell shell对象
 * @param length 删除长度
 */
void shellDeleteCommandLine(shell_t *shell, unsigned char length)
{
    while (length--)
    {
        shellWriteString(shell, "\b \b");
    }
}

/**
 * @brief shell 删除字节
 *
 * @param shell shell对象
 * @param direction 删除方向 {@code 1}删除光标前字符 {@code -1}删除光标处字符
 */
void shellDeleteByte(shell_t *shell, signed char direction)
{
    char offset = (direction == -1) ? 1 : 0;

    if ((shell->parser.cursor == 0 && direction == 1) || (shell->parser.cursor == shell->parser.length && direction == -1))
    {
        return;
    }
    if (shell->parser.cursor == shell->parser.length && direction == 1)
    {
        shell->parser.cursor--;
        shell->parser.length--;
        shell->parser.buffer[shell->parser.length] = 0;
        shellDeleteCommandLine(shell, 1);
    }
    else
    {
        for (short i = offset; i < shell->parser.length - shell->parser.cursor; i++)
        {
            shell->parser.buffer[shell->parser.cursor + i - 1] =
                shell->parser.buffer[shell->parser.cursor + i];
        }
        shell->parser.length--;
        if (!offset)
        {
            shell->parser.cursor--;
            shellWriteByte(shell, '\b');
        }
        shell->parser.buffer[shell->parser.length] = 0;
        for (short i = shell->parser.cursor; i < shell->parser.length; i++)
        {
            shellWriteByte(shell, shell->parser.buffer[i]);
        }
        shellWriteByte(shell, ' ');
        for (short i = shell->parser.length - shell->parser.cursor + 1; i > 0; i--)
        {
            shellWriteByte(shell, '\b');
        }
    }
}

/**
 * @brief shell写命令提示符
 *
 * @param shell shell对象
 * @param newline 新行
 *
 */
static void shellWritePrompt(shell_t *shell, unsigned char newline)
{
    if (shell->status.isChecked)
    {
        if (newline)
        {
            shellWriteString(shell, "\r\n");
        }
        if (shell->system.state == 0)
            shellWriteString(shell, shell->permission ? "User:" : "Root:");
    }
    else
    {
        shellWriteString(shell, "Please input password:");
    }
}

/**
 * @brief int转16进制字符串
 *
 * @param value 数值
 * @param buffer 缓冲
 *
 * @return signed char 转换后有效数据长度
 */
signed char shellToHex(unsigned int value, char *buffer)
{
    char byte;
    unsigned char i = 8;
    buffer[8] = 0;
    while (value)
    {
        byte = value & 0x0000000F;
        buffer[--i] = (byte > 9) ? (byte + 87) : (byte + 48);
        value >>= 4;
    }
    return 8 - i;
}

/**
 * @brief shell获取命令名
 *
 * @param command 命令
 * @return const char* 命令名
 */
static const char *shellGetCommandName(shell_cmd_t *command)
{
    const char *unknown = "";
    switch (command->attr.attrs.type.main)
    {
    case KEYCMD:
        return command->cmd.key.name ? command->cmd.key.name : unknown;
    case VARCMD:
        return command->cmd.var.name ? command->cmd.var.name : unknown;
    case FUNCMD:
        return command->cmd.func.name ? command->cmd.func.name : unknown;
    case SYSCMD:
        return command->cmd.sys.name ? command->cmd.sys.name : unknown;
    default:
        return unknown;
    }
}

/**
 * @brief shell 写命令描述字符串
 *
 * @param shell shell对象
 * @param string 字符串数据
 *
 * @return unsigned short 写入字符的数量
 */
static unsigned short shellWriteCommandDesc(shell_t *shell, const char *string)
{
    unsigned short count = 0;
    const char *p = string;
    assertAction(shell->write, return 0);
    while (*p && *p != '\r' && *p != '\n')
    {
        p++;
        count++;
    }

    if (count > 36)
    {
        shell->write((const uint8_t *)string, 36);
        shellWriteString(shell, "...");
    }
    else
    {
        shell->write((const uint8_t *)string, count);
    }
    return count > 36 ? 36 : 39;
}

/**
 * @brief shell获取命令描述
 *
 * @param command 命令
 * @return const char* 命令描述
 */
static const char *shellGetCommandDesc(shell_cmd_t *command)
{
    const char *unknown = "";
    switch (command->attr.attrs.type.main)
    {
    case KEYCMD:
        return command->cmd.key.desc ? command->cmd.key.desc : unknown;
    case VARCMD:
        return command->cmd.var.desc ? command->cmd.var.desc : unknown;
    case FUNCMD:
        return command->cmd.func.desc ? command->cmd.func.desc : unknown;
    case SYSCMD:
        return command->cmd.sys.desc ? command->cmd.sys.desc : unknown;
    default:
        return unknown;
    }
}

static unsigned short shellStringCompare(char *dest, char *src)
{
    unsigned short match = 0;
    unsigned short i = 0;

    while (*(dest + i) && *(src + i))
    {
        if (*(dest + i) != *(src + i))
        {
            break;
        }
        match++;
        i++;
    }
    return match;
}

/**
 * @brief shell字符串复制
 *
 * @param dest 目标字符串
 * @param src 源字符串
 * @return unsigned short 字符串长度
 */
static unsigned short shellStringCopy(char *dest, char *src)
{
    unsigned short count = 0;
    while (*(src + count))
    {
        *(dest + count) = *(src + count);
        count++;
    }
    *(dest + count) = 0;
    return count;
}

/**
 * @brief shell 清空命令行输入
 *
 * @param shell shell对象
 */
void shellClearCommandLine(shell_t *shell)
{
    for (short i = shell->parser.length - shell->parser.cursor; i > 0; i--)
    {
        shellWriteByte(shell, ' ');
    }
    shellDeleteCommandLine(shell, shell->parser.length);
}

#if SHELL_HISTORY_MAX_NUMBER > 0
/**
 * @brief shell历史记录添加
 *
 * @param shell shell对象
 */
static void shellHistoryAdd(shell_t *shell)
{
    shell->history.offset = 0;
    if (shell->history.number > 0 && strcmp(shell->history.item[(shell->history.record == 0 ? SHELL_HISTORY_MAX_NUMBER : shell->history.record) - 1],
                                            (const char *)shell->parser.buffer) == 0)
    {
        return;
    }
    if (shellStringCopy(shell->history.item[shell->history.record],
                        (char *)shell->parser.buffer) != 0)
    {
        shell->history.record++;
    }
    if (++shell->history.number > SHELL_HISTORY_MAX_NUMBER)
    {
        shell->history.number = SHELL_HISTORY_MAX_NUMBER;
    }
    if (shell->history.record >= SHELL_HISTORY_MAX_NUMBER)
    {
        shell->history.record = 0;
    }
}

/**
 * @brief shell历史记录查找
 *
 * @param shell shell对象
 * @param dir 方向 {@code <0}往上查找 {@code >0}往下查找
 */
static void shellHistory(shell_t *shell, signed char dir)
{
    if (dir > 0)
    {
        if (shell->history.offset-- <=
            -((shell->history.number > shell->history.record) ? shell->history.number : shell->history.record))
        {
            shell->history.offset = -((shell->history.number > shell->history.record)
                                          ? shell->history.number
                                          : shell->history.record);
        }
    }
    else if (dir < 0)
    {
        if (++shell->history.offset > 0)
        {
            shell->history.offset = 0;
            return;
        }
    }
    else
    {
        return;
    }
    shellClearCommandLine(shell);
    if (shell->history.offset == 0)
    {
        shell->parser.cursor = shell->parser.length = 0;
    }
    else
    {
        if ((shell->parser.length = shellStringCopy((char *)shell->parser.buffer,
                                                    shell->history.item[(shell->history.record + SHELL_HISTORY_MAX_NUMBER + shell->history.offset) % SHELL_HISTORY_MAX_NUMBER])) == 0)
        {
            return;
        }
        shell->parser.cursor = shell->parser.length;
        shellWriteString(shell, (const char *)shell->parser.buffer);
    }
}
#endif /** SHELL_HISTORY_MAX_NUMBER > 0 */

void shellListItem(shell_t *shell, shell_cmd_t *item)
{
    short spaceLength;

    spaceLength = 22 - shellWriteString(shell, shellGetCommandName(item));
    spaceLength = (spaceLength > 0) ? spaceLength : 4;
    do
    {
        shellWriteByte(shell, ' ');
    } while (--spaceLength);

    shellWriteString(shell, "  ");
    shellWriteCommandDesc(shell, shellGetCommandDesc(item));
    shellWriteString(shell, "\r\n");
}

void shellListKey(shell_t *shell)
{
    shell_cmd_t *current = (shell_cmd_t *)shell->head;
    shellWriteString(shell, "\r\nKey List:\r\n");
    while (current)
    {
        if (current->attr.attrs.type.main == KEYCMD && shellCheckPermission(shell, current) && current->attr.attrs.helpDisplay)
        {
            shellListItem(shell, current);
        }
        current = current->next;
    }
}

/**
 * @brief shell列出可执行命令
 *
 * @param shell shell对象
 */
void shellListCommand(shell_t *shell)
{
    shell_cmd_t *current = (shell_cmd_t *)shell->head;
    shellWriteString(shell, "\r\nFunction List:\r\n");
    while (current)
    {
        if (current->attr.attrs.type.main == FUNCMD && shellCheckPermission(shell, current) && current->attr.attrs.helpDisplay)
        {
            shellListItem(shell, current);
        }
        current = current->next;
    }
}

/**
 * @brief shell列出变量
 *
 * @param shell shell对象
 */
void shellListVar(shell_t *shell)
{
    shell_cmd_t *current = (shell_cmd_t *)shell->head;
    shellWriteString(shell, "\r\nVar List:\r\n");
    while (current)
    {
        if (current->attr.attrs.type.main == VARCMD && shellCheckPermission(shell, current) && current->attr.attrs.helpDisplay)
        {
            shellListItem(shell, current);
        }
        current = current->next;
    }
}

void shellListSys(shell_t *shell)
{
    shell_cmd_t *current = (shell_cmd_t *)shell->head;
    shellWriteString(shell, "\r\nSys List:\r\n");
    while (current)
    {
        if (current->attr.attrs.type.main == SYSCMD && shellCheckPermission(shell, current) && current->attr.attrs.helpDisplay)
        {
            shellListItem(shell, current);
        }
        current = current->next;
    }
}

void shellListAll(shell_t *shell)
{
    shellWriteString(shell, "\033[32m");
    if (shell->keyListFlag)
        shellListKey(shell);
    if (shell->varListFlag)
        shellListVar(shell);
    if (shell->sysListFlag)
        shellListSys(shell);
    if (shell->funcListFlag)
        shellListCommand(shell);
    shellWriteString(shell, "\033[0m");
}

// 系统基础指令
void _shellRight(shell_t *shell)
{
    if (shell->parser.cursor < shell->parser.length)
    {
        shellWriteByte(shell, shell->parser.buffer[shell->parser.cursor++]);
    }
}
shell_cmd_t shellRight = {.attr.attrs.type.main = KEYCMD, .cmd.key.value = KEYRIGHTANSICODE, .cmd.key.function = _shellRight};
void _shellLeft(shell_t *shell)
{
    if (shell->parser.cursor > 0)
    {
        shellWriteByte(shell, '\b');
        shell->parser.cursor--;
    }
}
shell_cmd_t shellLeft = {.attr.attrs.type.main = KEYCMD, .cmd.key.value = KEYLEFTANSICODE, .cmd.key.function = _shellLeft, .next = &shellRight};

void _shellBackspace(shell_t *shell)
{
    shellDeleteByte(shell, 1);
}
shell_cmd_t shelBackspace1 = {.attr.attrs.type.main = KEYCMD, .cmd.key.value = 0x08000000, .cmd.key.function = _shellBackspace, .next = &shellLeft};
shell_cmd_t shelBackspace2 = {.attr.attrs.type.main = KEYCMD, .cmd.key.value = 0x7F000000, .cmd.key.function = _shellBackspace, .next = &shelBackspace1};

void _shellDelete(shell_t *shell)
{
    _shellLeft(shell);
    shellDeleteByte(shell, -1);
}
shell_cmd_t shelDelete = {.attr.attrs.type.main = KEYCMD, .cmd.key.value = 0x1B5B337E, .cmd.key.function = _shellDelete, .next = &shelBackspace2};

void _shellTab(shell_t *shell)
{
    unsigned short maxMatch = shell->parser.bufferSize;
    shell_cmd_t *lastMatchIndex = 0;
    unsigned short matchNum = 0;
    unsigned short length;

    assertAction(shell->status.isChecked, return);

    // 便利所有命令，寻找匹配项
    if (shell->parser.length == 0)
    {
        shellListAll(shell);
        shellWritePrompt(shell, 1);
    }

    else if (shell->parser.length > 0)
    {
        shell->parser.buffer[shell->parser.length] = 0;
        shell_cmd_t *current = (shell_cmd_t *)shell->head;
        while (current)
        {
            if (shellCheckPermission(shell, current) && (current->attr.attrs.type.main != KEYCMD) &&
                shellStringCompare((char *)shell->parser.buffer, (char *)shellGetCommandName(current)) == shell->parser.length)
            {
                if (matchNum != 0)
                {
                    if (matchNum == 1)
                    {
                        shellWriteString(shell, "\r\n");
                    }
                    shellListItem(shell, lastMatchIndex);
                    length = shellStringCompare((char *)shellGetCommandName(lastMatchIndex),
                                                (char *)shellGetCommandName(current));
                    maxMatch = (maxMatch > length) ? length : maxMatch;
                }
                lastMatchIndex = current;
                matchNum++;
            }
            current = current->next;
        }

        if (matchNum == 0)
        {
            return;
        }
        if (matchNum == 1)
        {
            shellClearCommandLine(shell);
        }
        if (matchNum != 0)
        {
            shell->parser.length =
                shellStringCopy((char *)shell->parser.buffer,
                                (char *)shellGetCommandName(lastMatchIndex));
        }
        if (matchNum > 1)
        {
            shellListItem(shell, lastMatchIndex);
            shellWritePrompt(shell, 0);
            shell->parser.length = maxMatch;
        }
        shell->parser.buffer[shell->parser.length] = 0;
        shell->parser.cursor = shell->parser.length;
        shellWriteString(shell, (char *)shell->parser.buffer);
    }
}
shell_cmd_t shellTab = {.attr.attrs.type.main = KEYCMD, .cmd.key.value = 0x09000000, .cmd.key.function = _shellTab, .next = &shelDelete};

void _shellUp(shell_t *shell)
{
#if SHELL_HISTORY_MAX_NUMBER > 0
    assertAction(shell->status.isChecked, return);
    shellHistory(shell, 1);
#endif
}
shell_cmd_t shellUp = {.attr.attrs.type.main = KEYCMD, .cmd.key.value = 0x1B5B4100, .cmd.key.function = _shellUp, .next = &shellTab};

void _shellDown(shell_t *shell)
{
#if SHELL_HISTORY_MAX_NUMBER > 0
    assertAction(shell->status.isChecked, return);
    shellHistory(shell, -1);
#endif
}
shell_cmd_t shellDown = {.attr.attrs.type.main = KEYCMD, .cmd.key.value = 0x1B5B4200, .cmd.key.function = _shellDown, .next = &shellUp};

void shellExec(shell_t *shell);
void _shellEnter(shell_t *shell)
{
    shellExec(shell);
    shellWritePrompt(shell, 1);
}
shell_cmd_t shellEnter1 = {.attr.attrs.type.main = KEYCMD, .cmd.key.value = 0x0A000000, .cmd.key.function = _shellEnter, .next = &shellDown};
shell_cmd_t shellEnter2 = {.attr.attrs.type.main = KEYCMD, .cmd.key.value = 0x0D000000, .cmd.key.function = _shellEnter, .next = &shellEnter1};
// 系统基础指令结束

// 函数命令解析
unsigned char pairedChars[][2] = {
    {'\"', '\"'},

};

int shellSplit(char *string, unsigned short strLen, char *array[], char splitKey, short maxNum)
{
    unsigned char record = 1;
    unsigned char pairedLeft[16] = {0};
    unsigned char pariedCount = 0;
    int count = 0;

    for (short i = 0; i < maxNum; i++)
    {
        array[i] = NULL;
    }

    for (unsigned short i = 0; i < strLen; i++)
    {
        if (pariedCount == 0)
        {
            if (string[i] != splitKey && record == 1 && count < maxNum)
            {
                array[count++] = &(string[i]);
                record = 0;
            }
            else if ((string[i] == splitKey || string[i] == ' ') && record == 0)
            {
                string[i] = 0;
                if (string[i + 1] != ' ')
                {
                    record = 1;
                }
                continue;
            }
        }

        for (unsigned char j = 0; j < sizeof(pairedChars) / 2; j++)
        {
            if (pariedCount > 0 && string[i] == pairedChars[j][1] && pairedLeft[pariedCount - 1] == pairedChars[j][0])
            {
                --pariedCount;
                break;
            }
            else if (string[i] == pairedChars[j][0])
            {
                pairedLeft[pariedCount++] = pairedChars[j][0];
                pariedCount &= 0x0F;
                break;
            }
        }

        if (string[i] == '\\' && string[i + 1] != 0)
        {
            i++;
        }
    }
    return count;
}

/// @brief shell 解析参数
/// @param shell shell对象
static void shellParserParam(shell_t *shell)
{
    shell->parser.paramCount =
        shellSplit((char *)shell->parser.buffer, shell->parser.length,
                   shell->parser.param, ' ', SHELL_PARAMETER_MAX_NUMBER);
}

/**
 * @brief shell匹配命令
 *
 * @param shell shell对象
 * @param cmd 命令
 * @param base 匹配命令表基址
 * @param compareLength 匹配字符串长度
 * @return ShellCommand* 匹配到的命令
 */
shell_cmd_t *shellSeekCommand(shell_t *shell,
                              const char *cmd,
                              shell_cmd_t *base,
                              unsigned short compareLength)
{
    const char *name;
    shell_cmd_t *current = base;
    while (current)
    {
        if (current->attr.attrs.type.main == KEYCMD || shellCheckPermission(shell, current) == 0)
        {
            current = current->next;
            continue;
        }
        name = shellGetCommandName(current);
        if (!compareLength)
        {
            if (strcmp(cmd, name) == 0)
            {
                return current;
            }
        }
        else
        {
            if (strncmp(cmd, name, compareLength) == 0)
            {
                return current;
            }
        }
        current = current->next;
    }
    return NULL;
}

/**
 * @brief shell去除字符串参数头尾的双引号
 *
 * @param shell shell对象
 */
static void shellRemoveParamQuotes(shell_t *shell)
{
    unsigned short paramLength;
    for (unsigned short i = 0; i < shell->parser.paramCount; i++)
    {
        if (shell->parser.param[i][0] == '\"')
        {
            shell->parser.param[i][0] = 0;
            shell->parser.param[i] = &shell->parser.param[i][1];
        }
        paramLength = strlen(shell->parser.param[i]);
        if (shell->parser.param[i][paramLength - 1] == '\"')
        {
            shell->parser.param[i][paramLength - 1] = 0;
        }
    }
}

/**
 * @brief int转10进制字符串
 *
 * @param value 数值
 * @param buffer 缓冲
 *
 * @return signed char 转换后有效数据长度
 */
signed char shellToDec(int value, char *buffer)
{
    unsigned char i = 11;
    int v = value;
    if (value < 0)
    {
        v = -value;
    }
    buffer[11] = 0;
    while (v)
    {
        buffer[--i] = v % 10 + 48;
        v /= 10;
    }
    if (value < 0)
    {
        buffer[--i] = '-';
    }
    if (value == 0)
    {
        buffer[--i] = '0';
    }
    return 11 - i;
}

/**
 * @brief shell写返回值
 *
 * @param shell shell对象
 * @param value 返回值
 */
static void shellWriteReturnValue(shell_t *shell, int value)
{
    char buffer[12] = "00000000000";
    shellWriteString(shell, "Return: ");
    shellWriteString(shell, &buffer[11 - shellToDec(value, buffer)]);
    shellWriteString(shell, ", 0x");
    for (short i = 0; i < 11; i++)
    {
        buffer[i] = '0';
    }
    shellToHex(value, buffer);
    shellWriteString(shell, buffer);
    shellWriteString(shell, "\r\n");
}

static void shellWriteReturnValueFloat(shell_t *shell, int value)
{
    char buffer[12] = "00000000000";
    shellWriteString(shell, "Return: ");
    snprintf(buffer, sizeof(buffer), "%11.7g", *(float *)&value);
    shellWriteString(shell, buffer);
    shellWriteString(shell, "\r\n");
}

static void shellWriteReturnValueString(shell_t *shell, int value)
{
    shellWriteString(shell, "Return: ");
    shellWriteString(shell, (char *)value);
    shellWriteString(shell, "\r\n");
}

static void shellWriteReturn(shell_t *shell, int value, unsigned char display)
{
    if (display == SHELL_TYPE_FUNC_RETURN_INT)
    {
        shellWriteReturnValue(shell, value);
    }
    else if (display == SHELL_TYPE_FUNC_RETURN_FLOAT)
    {
        shellWriteReturnValueFloat(shell, value);
    }
    else if (display == SHELL_TYPE_FUNC_RETURN_STRING)
    {
        shellWriteReturnValueString(shell, value);
    }
#if SHELL_KEEP_RETURN_VALUE == 1
    shell->retVal = value;
#endif
}

/**
 * @brief 获取期待的参数个数
 *
 * @param signature 函数签名
 *
 * @return int 参数个数
 */
int shellGetParamNumExcept(const char *signature)
{
    int num = 0;
    const char *p = signature;

    while (*p)
    {
        if (*p)
        {
            p++;
            num++;
        }
    }
    return num;
}

/**
 * @brief 获取下一个参数类型
 *
 * @param signature 函数签名
 * @param index 参数遍历在签名中的起始索引
 * @param type 获取到的参数类型
 *
 * @return int 下一个参数在签名中的索引
 */
int shellGetNextParamType(const char *signature, int index, char *type)
{
    const char *p = signature + index;
    if (*p != 0)
    {
        *type++ = *p;
        index++;
    }
    *type = '\0';
    return index;
}

/**
 * @brief 解析字符参数
 *
 * @param string 字符串参数
 * @return char 解析出的字符
 */
static char shellExtParseChar(char *string)
{
    char *p = (*string == '\'') ? (string + 1) : string;
    char value = 0;

    if (*p == '\\')
    {
        switch (*(p + 1))
        {
        case 'b':
            value = '\b';
            break;
        case 'r':
            value = '\r';
            break;
        case 'n':
            value = '\n';
            break;
        case 't':
            value = '\t';
            break;
        case '0':
            value = 0;
            break;
        default:
            value = *(p + 1);
            break;
        }
    }
    else
    {
        value = *p;
    }
    return value;
}

/**
 * @brief 判断数字进制
 *
 * @param string 参数字符串
 * @return ShellNumType 进制
 */
static ShellNumType shellExtNumType(char *string)
{
    char *p = string;
    ShellNumType type = NUM_TYPE_DEC;

    if ((*p == '0') && ((*(p + 1) == 'x') || (*(p + 1) == 'X')))
    {
        type = NUM_TYPE_HEX;
    }
    else if ((*p == '0') && ((*(p + 1) == 'b') || (*(p + 1) == 'B')))
    {
        type = NUM_TYPE_BIN;
    }
    else if (*p == '0')
    {
        type = NUM_TYPE_OCT;
    }

    while (*p++)
    {
        if (*p == '.' && *(p + 1) != 0)
        {
            type = NUM_TYPE_FLOAT;
            break;
        }
    }

    return type;
}

/**
 * @brief 解析数字参数
 *
 * @param string 字符串参数
 * @return size_t 解析出的数字
 */
static size_t shellExtParseNumber(char *string)
{
    ShellNumType type = NUM_TYPE_DEC;
    signed char sign = 1;
    float valueFloat = 0.0;
    if (*string == '-')
    {
        sign = -1;
    }

    type = shellExtNumType(string + ((sign == -1) ? 1 : 0));

    switch ((char)type)
    {
    case NUM_TYPE_DEC:
        return (size_t)strtol(string, NULL, 10);
    case NUM_TYPE_HEX:
        return (size_t)strtol(string, NULL, 16);
    case NUM_TYPE_OCT:
        return (size_t)strtol(string, NULL, 8);
    case NUM_TYPE_BIN:
        return (size_t)strtol(string + 2, NULL, 2);
    case NUM_TYPE_FLOAT:
        valueFloat = strtof(string, NULL);
        return *((size_t *)(&valueFloat));
    default:
        return 0;
    }
}

/**
 * @brief 解析字符串参数
 *
 * @param string 字符串参数
 * @return char* 解析出的字符串
 */
static char *shellExtParseString(char *string)
{
    char *p = string;
    unsigned short index = 0;

    if (*string == '\"')
    {
        p = ++string;
    }

    while (*p)
    {
        if (*p == '\\')
        {
            *(string + index) = shellExtParseChar(p);
            p++;
        }
        else if (*p == '\"')
        {
            *(string + index) = 0;
        }
        else
        {
            *(string + index) = *p;
        }
        p++;
        index++;
    }
    *(string + index) = 0;
    return string;
}

/**
 * @brief 解析变量参数
 *
 * @param shell shell对象
 * @param var 变量
 * @param result 解析结果
 *
 * @return int 0 解析成功 --1 解析失败
 */
static int shellExtParseVar(shell_t *shell, char *var, size_t *result)
{
    shell_cmd_t *command = shellSeekCommand(shell,
                                            var + 1,
                                            shell->head,
                                            0);
    if (command)
    {
        *result = (size_t)command->cmd.var.value;
        return 0;
    }
    else
    {
        return -1;
    }
}

/**
 * @brief 解析参数
 *
 * @param shell shell对象
 * @param string 参数
 * @param type 参数类型
 * @param result 解析结果
 *
 * @return int 0 解析成功 --1 解析失败
 */
int shellExtParsePara(shell_t *shell, char *string, char *type, size_t *result)
{
    if (type == NULL || (*string == '$' && *(string + 1)))
    {
        if (*string == '\\' && *(string + 1))
        {
            *result = (size_t)shellExtParseChar(string);
            return 0;
        }
        else if (*string == '-' || (*string >= '0' && *string <= '9'))
        {
            *result = shellExtParseNumber(string);
            return 0;
        }
        else if (*string == '$' && *(string + 1))
        {
            return shellExtParseVar(shell, string, result);
        }
        else if (*string == '{')
        {
            *result = (size_t)string;
            return 0;
        }
        else if (*string)
        {
            *result = (size_t)shellExtParseString(string);
            return 0;
        }
    }
    return -1;
}

/**
 * @brief shell校验密码
 *
 * @param shell shell对象
 */
static void shellCheckPassword(shell_t *shell)
{
    if (strcmp((const char *)shell->parser.buffer, shell->system.password) == 0)
    {
        shell->system.state = 1;
        shellWriteString(shell, "\033[2K\r");
        shellWriteString(shell, "System Mode\r\n");
    }
    else
    {
        shellWriteString(shell, "\r\npassword error\r\n");
    }
    shell->parser.length = 0;
    shell->parser.cursor = 0;
    shell->status.isChecked = 1;
}

int shellExtRun(shell_t *shell, shell_cmd_t *command, int argc, char *argv[])
{
    int ret = 0;
    size_t params[SHELL_PARAMETER_MAX_NUMBER] = {0};
    int paramNum = command->attr.attrs.paramNum > (argc - 1) ? command->attr.attrs.paramNum : (argc - 1);

    for (int i = 0; i < argc - 1; i++)
    {
        if (shellExtParsePara(shell, argv[i + 1], NULL, &params[i]) != 0)
        {
            return -1;
        }
    }
    switch (paramNum)
    {
#if SHELL_PARAMETER_MAX_NUMBER >= 1
    case 0:
        ret = command->cmd.func.function();
        break;
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 1 */
#if SHELL_PARAMETER_MAX_NUMBER >= 2
    case 1:
    {
        int (*func)(size_t) = command->cmd.func.function;
        ret = func(params[0]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 2 */
#if SHELL_PARAMETER_MAX_NUMBER >= 3
    case 2:
    {
        int (*func)(size_t, size_t) = command->cmd.func.function;
        ret = func(params[0], params[1]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 3 */
#if SHELL_PARAMETER_MAX_NUMBER >= 4
    case 3:
    {
        int (*func)(size_t, size_t, size_t) = command->cmd.func.function;
        ret = func(params[0], params[1], params[2]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 4 */
#if SHELL_PARAMETER_MAX_NUMBER >= 5
    case 4:
    {
        int (*func)(size_t, size_t, size_t, size_t) = command->cmd.func.function;
        ret = func(params[0], params[1], params[2], params[3]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 5 */
#if SHELL_PARAMETER_MAX_NUMBER >= 6
    case 5:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t) = command->cmd.func.function;
        ret = func(params[0], params[1], params[2], params[3], params[4]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 6 */
#if SHELL_PARAMETER_MAX_NUMBER >= 7
    case 6:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t) = command->cmd.func.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 7 */
#if SHELL_PARAMETER_MAX_NUMBER >= 8
    case 7:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t, size_t) = command->cmd.func.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5], params[6]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 8 */
#if SHELL_PARAMETER_MAX_NUMBER >= 9
    case 8:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t) = command->cmd.func.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 9 */
#if SHELL_PARAMETER_MAX_NUMBER >= 10
    case 9:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
                    size_t) = command->cmd.func.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7],
                   params[8]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 10 */
#if SHELL_PARAMETER_MAX_NUMBER >= 11
    case 10:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
                    size_t, size_t) = command->cmd.func.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7],
                   params[8], params[9]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 11 */
#if SHELL_PARAMETER_MAX_NUMBER >= 12
    case 11:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
                    size_t, size_t, size_t) = command->cmd.func.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7],
                   params[8], params[9], params[10]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 12 */
#if SHELL_PARAMETER_MAX_NUMBER >= 13
    case 12:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
                    size_t, size_t, size_t, size_t) = command->cmd.func.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7],
                   params[8], params[9], params[10], params[11]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 13 */
#if SHELL_PARAMETER_MAX_NUMBER >= 14
    case 13:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
                    size_t, size_t, size_t, size_t, size_t) = command->cmd.func.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7],
                   params[8], params[9], params[10], params[11], params[12]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 14 */
#if SHELL_PARAMETER_MAX_NUMBER >= 15
    case 14:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
                    size_t, size_t, size_t, size_t, size_t, size_t) = command->cmd.func.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7],
                   params[8], params[9], params[10], params[11], params[12], params[13]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 15 */
#if SHELL_PARAMETER_MAX_NUMBER >= 16
    case 15:
    {
        int (*func)(size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
                    size_t, size_t, size_t, size_t, size_t, size_t, size_t) = command->cmd.func.function;
        ret = func(params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7],
                   params[8], params[9], params[10], params[11], params[12], params[13], params[14]);
        break;
    }
#endif /** SHELL_PARAMETER_MAX_NUMBER >= 16 */
    default:
        ret = -1;
        break;
    }
    return ret;
}

unsigned int shellRunCommand(shell_t *shell, shell_cmd_t *command)
{
    int returnValue = 0;
    if (command->attr.attrs.type.main == FUNCMD)
    {
        if (command->attr.attrs.type.addition == SHELL_TYPE_FUNC_MAIN)
        {
            shellRemoveParamQuotes(shell);
            int (*func)(int, char **) = command->cmd.func.function;
            returnValue = func(shell->parser.paramCount, shell->parser.param);
            shellWriteReturn(shell, returnValue, command->attr.attrs.type.returnDisplay);
        }
        else if (command->attr.attrs.type.addition == SHELL_TYPE_FUNC_FUNC_AUTO)
        {
            returnValue = shellExtRun(shell,
                                      command,
                                      shell->parser.paramCount,
                                      shell->parser.param);
            shellWriteReturn(shell, returnValue, command->attr.attrs.type.returnDisplay);
        }
    }
    else if (command->attr.attrs.type.main == VARCMD)
    {
        if (command->attr.attrs.type.addition == SHELL_TYPE_VAR_NUMBER)
        {
            if (shell->parser.paramCount == 2)
            {
                returnValue = shellExtParseNumber(shell->parser.param[1]);
                *(size_t *)command->cmd.var.value = returnValue;
                shellWriteReturn(shell, returnValue, SHELL_TYPE_FUNC_RETURN_INT);
                shellWriteReturn(shell, returnValue, SHELL_TYPE_FUNC_RETURN_FLOAT);
            }
            else
            {
                returnValue = *(size_t *)command->cmd.var.value;
                shellWriteReturn(shell, returnValue, SHELL_TYPE_FUNC_RETURN_INT);
                shellWriteReturn(shell, returnValue, SHELL_TYPE_FUNC_RETURN_FLOAT);
            }
        }
        else if (command->attr.attrs.type.addition == SHELL_TYPE_VAR_STRING)
        {
            if (shell->parser.paramCount == 2)
            {
                char *str = shellExtParseString(shell->parser.param[1]);
                strcpy(command->cmd.var.value, str);
            }
            else
            {
                returnValue = (size_t)command->cmd.var.value;
                shellWriteReturn(shell, returnValue, SHELL_TYPE_FUNC_RETURN_STRING);
            }
        }
    }
    else if (command->attr.attrs.type.main == SYSCMD)
    {
        shell->system.func = command->cmd.sys.function;
        shell->system.password = command->cmd.sys.password;
        shell->status.isChecked = 0;
    }
    return returnValue;
}

/// @brief shell执行命令
/// @param shell shell对象
void shellExec(shell_t *shell)
{
    if (shell->parser.length == 0)
    {
        return;
    }

    shell->parser.buffer[shell->parser.length] = 0;

    if (shell->status.isChecked)
    {
#if SHELL_HISTORY_MAX_NUMBER > 0
        shellHistoryAdd(shell);
#endif
        shellParserParam(shell);
        shell->parser.length = shell->parser.cursor = 0;
        if (shell->parser.paramCount == 0)
        {
            return;
        }
        shellWriteString(shell, "\r\n");

        shell_cmd_t *command = shellSeekCommand(shell,
                                                shell->parser.param[0],
                                                shell->head,
                                                0);
        if (command)
        {
            // 调用命令处理函数
            shellRunCommand(shell, command);
        }
        else
        {
            // 不存在命令
            shellWriteString(shell, "Command not Found\r\n");
        }
    }
    else
    {
        shellCheckPassword(shell);
    }
}

/// @brief shell普通输入
/// @param shell
/// @param data
void shellNormalInput(shell_t *shell, uint8_t data)
{
    shell->status.tabFlag = 0;
    shellInsertByte(shell, data);
}

void shellHandler(shell_t *shell, uint8_t data)
{
    assertAction(data, return);

    uint8_t keyByteOffset = 24;
    uint32_t keyFilter = 0x00000000;
    if ((shell->parser.keyValue & 0x0000FF00) != 0x00000000)
    {
        keyByteOffset = 0;
        keyFilter = 0xFFFFFF00;
    }
    else if ((shell->parser.keyValue & 0x00FF0000) != 0x00000000)
    {
        keyByteOffset = 8;
        keyFilter = 0xFFFF0000;
    }
    else if ((shell->parser.keyValue & 0xFF000000) != 0x00000000)
    {
        keyByteOffset = 16;
        keyFilter = 0xFF000000;
    }

    shell_cmd_t *current = (shell_cmd_t *)shell->head;

    while (current)
    {
        if (current->attr.attrs.type.main == KEYCMD && shellCheckPermission(shell, current))
        {
            if ((current->cmd.key.value & keyFilter) == shell->parser.keyValue &&              // 判断存储的键值是否匹配
                (current->cmd.key.value & (0xFF << keyByteOffset)) == (data << keyByteOffset)) // 判断当前输入的字节是否匹配
            {
                shell->parser.keyValue |= data << keyByteOffset; // 存储
                if (keyByteOffset == 0 || (current->cmd.key.value & (0xFF << (keyByteOffset - 8))) == 0)
                {
                    if (current->cmd.key.function)
                    {
                        current->cmd.key.function(shell);
                    }
                    shell->parser.keyValue = 0;
                }
                return;
            }
        }
        current = current->next;
    }

    shell->parser.keyValue = 0x00000000;
    shellNormalInput(shell, data);
}

/**
 * @brief shell格式化输出
 *
 * @param shell shell对象
 * @param fmt 格式化字符串
 * @param ... 参数
 */
void shellPrintf(shell_t *shell, const char *fmt, ...)
{
    char buffer[SHELL_PRINT_BUFFER];
    va_list vargs;
    int len;

    assertAction(shell, return);

    va_start(vargs, fmt);
    len = vsnprintf(buffer, SHELL_PRINT_BUFFER, fmt, vargs);
    va_end(vargs);
    if (len > SHELL_PRINT_BUFFER)
    {
        len = SHELL_PRINT_BUFFER;
    }
    if(!shell->system.state)
    {
        shellWriteString(shell, "\033[2K\r");
    }
    shell->write((uint8_t *)buffer, len);
    if(!shell->system.state)
    {
        shellWritePrompt(shell, 0);
        if (shell->parser.length > 0)
        {
            shellWriteString(shell, (const char *)shell->parser.buffer);
            for (short i = 0; i < shell->parser.length - shell->parser.cursor; i++)
            {
                shellWriteByte(shell, '\b');
            }
        }
    }
}

/// @brief 运行Shell任务
void shellTask(void *param)
{
    shell_t *shell = (shell_t *)param;
    uint8_t data;
#if SHELL_TASK_WHILE
    while (1)
    {
#endif
        if (shell->system.state)
        {
            if (shell->system.func)
            {
                shell->system.func(shell);
            }
        }
        else
        {
            if (shell->read)
            {
                if (shell->read(&data, 1) == 1)
                {
                    shellHandler(shell, data);
                }
            }
        }

#if SHELL_TASK_WHILE
    }
#endif
}
