# cJSON简单使用手册

## 输出cJSON

- ### 创建cJSON类型文件
  ```c
  cJSON *root = cJSON_CreateObject();
  ```

- ### 添加对象到cJSON类型中

  - ##### 添加字符串

  ```c
  char string[] = "hello world";
  cJSON_AddStringToObject(child, "string", string);
  ```

  - ##### 添加数

  ```c
  int number_i = 100;
  double number_f = 15.5;
  cJSON_AddNumberToObject(root, "int", number_i);
  cJSON_AddNumberToObject(root, "double", number_f);
  ```

  - ##### 添加布尔

  ```c
  bool flag = false;
  cJSON_AddBoolToObject(root, "flag", flag);
  ```

  - ##### 添加NULL

  ```c
  cJSON_AddNullToObject(root, "null");
  ```

  - ##### 添加数组

  ```c
  cJSON *person = cJSON_AddArrayToObject(root, "person");
  cJSON_AddItemToArray(person, cJSON_CreateString("John"));
  cJSON_AddItemToArray(person, cJSON_CreateString("Mary"));
  cJSON_AddItemToArray(person, cJSON_CreateString("Tom"));
  ```

  - ##### 添加cJSON

  ```c
  cJSON *child = cJSON_AddObjectToObject(root, "child");
  ```

- ### 输出cJSON字符串

  ```c
  char *json_string = cJSON_Print(root);
  printf("%s\n", json_string);
  free(json_string);
  ```

- ### 释放内存

  ```c
  cJSON_Delete(root);
  ```

## 解析cJSON内容

- ### 编译字符串为cJSON


```c
cJSON *root = cJSON_Parse(json_string);
```
- ### 找到所需对象

  ```c
  cJSON *object = cJSON_GetObjectItemCaseSensitive(root, "name");
  ```

- ### 按类型解析

  - ##### 解析JSON类型

    ```c
    if(object && cJSON_IsObject(object))
    {
    	cJSON *child_object = cJSON_GetObjectItemCaseSensitive(object, "child_name");
        if(child_object); // 对应类型判断
        // 后续操作...
    }
    ```

  - ##### 解析数值类型

    ```c
    if(object && cJSON_IsNumber(object))
    {
    	int number_i = object->valueint;
    	double number_d = object->valuedouble;
    	// 后续操作...
    }
    ```

  - ##### 解析字符串类型

    ```c
    if(cJSON_IsString(object))
    {
    	char *string = object->valuestring;
    	// 后续操作...
    }
    ```

  - ##### 解析布尔类型

    ```c
    if(cJSON_IsBool(object))
    {
    	bool flag = object->valueint;
        // 后续操作...
    }
    ```

  - ##### 解析null类型

    ```c
    if(cJSON_IsNull(object))
    {
    	// 后续操作...
    }
    ```

  - ##### 解析数组类型

    ```c
    if(cJSON_IsArray(object))
    {
    	cJSON *object_item;
    	cJSON_ArrayForEach(object_item,object)
    	{
    		// 后续操作...
    	}
    }
    ```

- ### 释放内存

  ```c
  cJSON_Delecte(root);
  ```
  



