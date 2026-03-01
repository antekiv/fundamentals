### TEMPLATE TYPE DEDUCTION
```cpp
// 1. Базовые Lvalue
int x = 10;                      // 1. Обычный int
const int cx = x;                // 2. Константный int
const int& rx = x;               // 3. Константная ссылка

// 2. Указатели (Lvalue)
char str[] = "Mut";
char* m = str;                   // 4. Изменяемый указатель (данные можно менять, указатель можно менять)
const char* s = "C++";           // 5. Указатель на константу (данные const, указатель можно менять)
const char* const p = s;         // 6. Конст. указатель на константу (данные const, указатель const)

// 3. Массивы и функции (Lvalue)
const char arr[10] = "Array";    // 7. Массив
void func(int);                  // 8. Функция

// 4. Rvalue (Временные объекты)
// std::move(x)                  // 9. xvalue (rvalue-ссылка типа int&&)
// 27                            // 10. prvalue (чистый литерал типа int)
```

---
| Вызов `f(...)` | Исходный тип аргумента | Выведенный `T` | Итоговый `ParamType` | Механика вывода |
| :--- | :--- | :--- | :--- | :--- |
| 🟦 **`ParamType` = `T` (Передача по значению — всегда создается копия)** | | | | |
| `f(x)` | `int` | **`int`** | `int` | Копия. |
| `f(cx)`| `const int` | **`int`** | `int` | Top-level `const` снят (копию менять можно). |
| `f(rx)`| `const int&` | **`int`** | `int` | Ссылка снята, `const` снят. |
| `f(m)` | `char*` | **`char*`** | `char*` | Копия указателя. |
| `f(s)` | `const char*` | **`const char*`** | `const char*` | Low-level `const` сохранен (данные защищены). |
| `f(p)` | `const char* const` | **`const char*`** | `const char*` | Top-level `const` (правый) снят с указателя. |
| `f(arr)`|`const char[10]` | **`const char*`** | `const char*` | **Распад (Decay)** массива в указатель. |
| `f(func)`|`void(int)` | **`void(*)(int)`** | `void(*)(int)` | **Распад (Decay)** функции в указатель. |
| `f(std::move(x))`| `int&&` | **`int`** | `int` | Передаем rvalue по значению -> обычная копия `int`. |
| `f(27)`| `int` | **`int`** | `int` | Литерал копируется в новый `int`. |
| | | | | |
| 🟩 **`ParamType` = `T&` (Передача по lvalue-ссылке — работа с оригиналом)** | | | | |
| `f(x)` | `int` | **`int`** | `int&` | Обычная ссылка. |
| `f(cx)`| `const int` | **`const int`** | `const int&` | `const` обязан войти в `T` для защиты данных. |
| `f(rx)`| `const int&` | **`const int`** | `const int&` | Ссылка снята, `const` вошел в `T`. |
| `f(m)` | `char*` | **`char*`** | `char*&` | Ссылка на изменяемый указатель. |
| `f(s)` | `const char*` | **`const char*`** | `const char*&` | Ссылка на указатель на константу. |
| `f(p)` | `const char* const` | **`const char* const`**| `const char* const&`| Ссылка на конст. указатель на константу. |
| `f(arr)`|`const char[10]` | **`const char[10]`** | `const char(&)[10]` | **Нет распада.** Ссылка на сам массив. |
| `f(func)`|`void(int)` | **`void(int)`** | `void(&)(int)` | **Нет распада.** Ссылка на функцию. |
| `f(std::move(x))`| `int&&` | ❌ *ОШИБКА* | ❌ *ОШИБКА* | Нельзя привязать `T&` к rvalue (xvalue). |
| `f(27)`| `int` | ❌ *ОШИБКА* | ❌ *ОШИБКА* | Нельзя привязать `T&` к rvalue (prvalue). |
| | | | | |
| 🟨 **`ParamType` = `const T&` (Константная ссылка — обещание не менять)** | | | | |
| `f(x)` | `int` | **`int`** | `const int&` | `const` берется из шаблона. |
| `f(cx)`| `const int` | **`int`** | `const int&` | `const` аргумента поглощен шаблоном. |
| `f(rx)`| `const int&` | **`int`** | `const int&` | Ссылка игнорируется, `const` поглощен. |
| `f(m)` | `char*` | **`char*`** | `char* const &` | ⚠️ Ссылка на константный указатель. Данные менять **МОЖНО**. |
| `f(s)` | `const char*` | **`const char*`** | `const char* const &` | ⚠️ Ссылка на конст. указатель на конст. данные. |
| `f(p)` | `const char* const` | **`const char*`** | `const char* const &` | `T` = `const char*`. Правый `const` берется из шаблона. |
| `f(arr)`|`const char[10]` | **`char[10]`** | `const char(&)[10]` | Итог: константная ссылка на массив. |
| `f(func)`|`void(int)` | **`void(int)`** | `void(&)(int)` | `const` игнорируется (функции не бывают `const`). |
| `f(std::move(x))`| `int&&` | **`int`** | `const int&` | Успех. Константная ссылка продлевает жизнь rvalue. |
| `f(27)`| `int` | **`int`** | `const int&` | Успех. Аналогично. |
| | | | | |
| 🟪 **`ParamType` = `T&&` (Пересылающая ссылка — идеальный приемник для всего)** | | | | |
| `f(x)` | `int` | **`int&`** | `int&` | Lvalue -> `T` стал ссылкой `int&`. Схлопывание: `int&` + `&&` = `int&`. |
| `f(cx)`| `const int` | **`const int&`** | `const int&` | Lvalue -> `T` стал `const int&`. Итог: `const int&`. |
| `f(rx)`| `const int&` | **`const int&`** | `const int&` | Аналогично, lvalue -> `const int&`. |
| `f(m)` | `char*` | **`char*&`** | `char*&` | Lvalue -> `char*&`. |
| `f(s)` | `const char*` | **`const char*&`** | `const char*&` | Lvalue -> `const char*&`. |
| `f(p)` | `const char* const` | **`const char* const&`**|`const char* const&`| Lvalue -> `const char* const&`. |
| `f(arr)`|`const char[10]` | **`const char(&)[10]`**|`const char(&)[10]`| Lvalue массив. |
| `f(func)`|`void(int)` | **`void(&)(int)`** | `void(&)(int)` | Lvalue функция. |
| `f(std::move(x))`| `int&&` | **`int`** | `int&&` | Rvalue -> `T` выводится чистым (`int`). Итог: `int&&` (rvalue-ссылка). |
| `f(27)`| `int` | **`int`** | `int&&` | Rvalue -> `T` чистый (`int`). Итог: `int&&` (rvalue-ссылка). |
| | | | | |
| 🟧 **`ParamType` = `T*` (Передача по указателю)** — *Для не-указателей передаем их адрес `&`* | | | | |
| `f(&x)`| `int*` | **`int`** | `int*` | Очевидно. |
| `f(&cx)`| `const int*` | **`const int`** | `const int*` | `const` входит в `T`, так как в `ParamType` его нет. |
| `f(&rx)`| `const int*` | **`const int`** | `const int*` | Взятие адреса от ссылки дает адрес объекта. |
| `f(m)` | `char*` | **`char`** | `char*` | Очевидно. |
| `f(s)` | `const char*` | **`const char`** | `const char*` | `const` данных входит в `T`. |
| `f(p)` | `const char* const` | **`const char`** | `const char*` | Top-level `const` (указателя) отбрасывается (копия). |
| `f(arr)`|`const char[10]` | **`const char`** | `const char*` | **Распад.** Массив `arr` распадается в `const char*`. |
| `f(func)`|`void(int)` | **`void(int)`** | `void(*)(int)` | **Распад.** Функция распадается в `void(*)(int)`. |
| `f(&std::move...`| — | ❌ *ОШИБКА* | ❌ *ОШИБКА* | У rvalue нельзя взять адрес `&`. |
| `f(&27)`| — | ❌ *ОШИБКА* | ❌ *ОШИБКА* | Аналогично, у литерала нет адреса памяти. |

---

### Точечные нюансы, которые эта таблица проясняет окончательно:

1. **Разница `p` в `T&` и `const T&`**:
   * При передаче `p` (`const char* const`) в **`T&`**, `T` вынужден впитать всё: `const char* const`.
   * При передаче `p` в **`const T&`**, внешний `const` берется из шаблона, поэтому `T` выводится просто как `const char*`.
2. **Игнорирование `const` для функций**:
   В блоке `const T&` при передаче `func`, компилятор выводит `T` как `void(int)`. Спецификатор становится `const void(&)(int)`, но C++ игнорирует `const` для ссылок на функции, оставляя просто `void(&)(int)`.
3. **Распад в `T*`**:
   Если шаблон ожидает указатель `T*`, а вы передаете массив `arr`, массив всё равно подвергается decay (распаду) до указателя на первый элемент, и только потом происходит сопоставление.