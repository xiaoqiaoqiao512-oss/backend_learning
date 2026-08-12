# MiniBackend 学习路线与任务书

## 1. 项目目标

这个项目的目标不是尽快堆出“注册、登录、JWT”等功能，而是通过亲手实现一个小型 C++ Web 服务，理解后端框架隐藏起来的核心流程：

```text
客户端
  -> TCP 连接
  -> HTTP 请求读取与解析
  -> Router 路由匹配
  -> Controller 参数校验
  -> Service 业务逻辑
  -> Repository 数据持久化
  -> HTTP 响应
```

最终目标：实现一个带用户注册、登录和认证功能的 REST API，并具有测试、日志、配置和基本并发能力。

> 学习原则：每个阶段必须做到“能运行、能解释、能测试”，再进入下一阶段。不要按天数赶进度。

---

## 2. 当前进度评估（2026-08-12）

当前主项目目录是 `MiniBackend/`，根目录下的 `http.cpp` 和 `src/` 是早期练习代码，不要和主项目混用。
`MiniBackend/docs/项目架构说明.md` 是新增的分层架构讲解文档，与本文档配合阅读。

### 已完成或已有雏形（相对 8-04 的进展）

- 分层调用已打通：`Server -> Router -> UserController -> UserService ->
  UserRepository -> User(model)`，Controller 通过构造函数注入 Service。
- 已引入 `nlohmann/json`（CMake `find_package` 管理），`Request` 增加 `body` 和
  `params`（路由参数）字段。
- `Response` 已修复：`HTTP/1.1 200 OK` 状态行合法，自动生成 `Content-Length` 与
  `Connection: close`；提供 `Response::json()` / `Response::error()` 工厂方法。
- 已实现 `sendAll()` 循环发送，`readRequest()` 循环读取（先读 `\r\n\r\n`，再按
  `Content-Length` 读满 body）。
- Router 已支持 `get()/post()` 注册、method 匹配、`:id` 路径参数
  （`request.params`）。
- 内存版 REST 雏形已实现：`POST /user` 创建用户、`GET /user/:id` 查询用户，
  不存在时返回 404 + JSON。
- 8-04 检查清单中的以下问题已修复：状态行空格、`Content-Length`/`Connection` 自动生成、
  `sendAll`、`recv` 循环、Parser 解析 headers/body、Router method 匹配、
  `Userservice.cpp` 大小写、未使用参数 warning（干净构建 0 warning）、
  静态 Service 改为构造注入、`"Unknow User"` 字符串改为结构化 404。

### 本次代码检查结果（2026-08-12，干净构建 + 实测）

在独立构建目录执行 `cmake -S MiniBackend -B /tmp/mb-verify-build && cmake --build
/tmp/mb-verify-build --clean-first`：**构建成功，无 warning**。

实测结果：正常请求（POST /user、GET /user/:id、404）响应文本合法；
但存在以下问题，按严重性排列：

1. **【严重】非法 JSON body 会杀死整个进程。**
   `UserController::createUser` 直接 `json::parse(request.body)`，没有 try-catch。
   实测 `curl -X POST localhost:8080/user -d 'not-json'` 后服务进程直接退出，
   后续所有请求全部失败。这是当前最严重的问题：**任何一个坏请求都能让服务下线**。
2. **【严重】非数字 id 会杀死整个进程。**
   `getUserById` 直接 `std::stoi(request.params.at("id"))`，实测
   `GET /user/abc` 同样导致进程退出。
3. **【重启失败】没有 `SO_REUSEADDR`。** 实测进程崩溃后立刻重启会出现
   `bind failed`（连接处于 TIME_WAIT），错误信息也不带 `errno`。
4. **query string 未拆分。** `GET /user/1?x=1` 会把 `1?x=1` 当成路径段参与匹配，
   实测返回 404；应先将 query 从 path 中拆出。
5. **header value 残留 `\r`。** `Http::parseRequest` 用 `std::getline`（按 `\n` 分隔）
   读头部，只去掉 value 的前导空格，没有去掉行尾 `\r`。目前 `readRequest` 恰好是
   裸字符串查找 `Content-Length` 所以能工作，但将来按 `request.headers` 取值会踩坑。
6. **没有 400 / 413。** 请求行格式无校验、无大小上限；收到
   `Transfer-Encoding: chunked` 也没有明确响应。
7. **Router 无 405、无重复路由检查。** method 不匹配时仍返回 404；重复注册同一路由
   不会报错。
8. **`/hello`、`/echo` 尚未恢复。** 当前只注册了 `/user` 相关路由，里程碑 2 的验收
   命令会得到 404。
9. **Router 兜底 404 无 Content-Type，body 是纯文本 `"404"`**，与其他 JSON 错误响应
   不一致。
10. **没有任何自动化测试。** Response、Parser、Router 的问题只能靠手动发现。
11. **工程卫生未做。** 仍无 `.gitignore`，`MiniBackend/build/` 下约 60 个 CMake
    生成文件被 Git 跟踪（8-12 的提交又带入了新的构建产物）。
12. **CMake 收尾项未做。** 未设置 `CMAKE_CXX_STANDARD_REQUIRED ON` 和
    `CMAKE_CXX_EXTENSIONS OFF`；warning 选项仍是全局 `add_compile_options`。
13. **`server_fd_` 无 RAII 保护**；系统调用错误信息不含 `errno`；`accept()` 失败仅
    `continue`，无日志。
14. **`UserService::count` 仍是 static 计数器**，id 生成逻辑藏在全局状态里。
15. **里程碑 4 差距仍在。** 无输入校验（缺 name 会抛异常崩溃）、错误格式是
    `{"error": ...}` 而任务书建议 `{"code", "message"}` 结构、无 `GET /users` 列表接口。

### 工程目录问题

- CMake 入口统一为 `MiniBackend/CMakeLists.txt`，根目录原 `CMakeLists.txt` 已删除，
  构建命令为 `cmake -S MiniBackend -B build`。
- `MiniBackend/build/` 构建产物仍在版本控制中，应建 `.gitignore` 后从索引移除；
  不要删除本地源码。
- `docs/项目架构说明.md` 已存在，其中 5.0 节关于“CMakeLists 第 32 行漏 `.cpp` 后缀”
  的描述已过时（现已写全），后续修改架构时注意同步该文档。
- 根目录 `src/`、`http.cpp` 是早期练习，继续开发时保持边界。

因此当前阶段应标记为：**里程碑 1 完成；里程碑 2 主链路完成但可靠性未验收
（崩溃问题 + 无 400/413 + 无测试）；里程碑 3 完成 method 匹配与 `:id` 路径参数
（缺 405/重复检查）；里程碑 4 内存版雏形已实现（缺输入校验、统一错误格式、列表接口
和测试）。**

---

## 3. 修正后的学习顺序

原计划中的方向基本正确，但需要作以下调整：

1. **先完成 HTTP 闭环，再写 Router。** 如果不能稳定读取请求并生成响应，路由层没有可靠输入输出。
2. **现在就加入 CMake 和测试。** 多文件项目不能等到最后才工程化；后续每个组件都应能单独测试。
3. **先学 JSON，再接数据库。** REST 接口先确定请求/响应格式，数据库才不会绑架上层设计。
4. **登录密码不能明文保存，JWT 也不要手写密码算法。** 学习项目应使用成熟库完成密码哈希、签名和校验。
5. **线程池应在功能正确、测试稳定后加入。** 并发会放大资源泄漏、竞态和异常处理问题。
6. **文件上传和 AI API 是扩展任务。** 它们不应挤占 HTTP、数据库、安全和测试等后端基础训练。
7. **任何 Handler 都必须有异常边界。** 这次实测证明：一个坏请求就能杀死整个进程。
   先学会“错误不扩散”，再谈功能。

建议按实际掌握情况用 4～6 周完成，不再限制为两周。

---

## 4. 里程碑路线

### 里程碑 1：TCP 与最小请求行（已完成）

应能解释：

- 监听 socket 与连接 socket 的区别。
- fd 为什么是整数，以及它对应的对象在哪里。
- HTTP、TCP、IP 分别负责什么。
- 为什么一次 `recv` 不等于一次 HTTP 请求。

保留早期代码作为学习记录，不继续在其中添加业务功能。

---

### 里程碑 2：完整的单连接 HTTP 闭环（主链路已通，可靠性未验收）

目标：执行 `curl -i http://127.0.0.1:8080/hello` 能收到合法响应，服务端对常见错误不会崩溃。

#### 任务 2.1：建立 CMake 构建（基本完成，待收尾）

`MiniBackend/CMakeLists.txt` 已满足：

- C++17、生成可执行文件 `minibackend`、登记 `src/` 下所有 `.cpp`、
  `-Wall -Wextra -Wpedantic`（注意：当前为全局选项）、链接 `nlohmann_json`。

当前收尾任务：

- 设置 `CMAKE_CXX_STANDARD_REQUIRED ON` 和 `CMAKE_CXX_EXTENSIONS OFF`。
- 让 warning 选项只对 `minibackend` target 生效，而不是全局生效。
- 添加 `.gitignore`，忽略 `build/`、可执行文件、编辑器缓存和运行时数据库。
- 将已经误提交的构建产物从 Git 索引移除；这只是停止跟踪，不应删除本地源码。

#### 任务 2.2：实现 `Response`（已完成）

`src/http/Response.h/.cpp` 已实现：

- 状态行 `HTTP/1.1 200 OK` 格式合法（含空格）。
- 自动生成 `Content-Length`（按 `body.size()` 字节数）与 `Connection: close`。
- 提供 `json()` 与 `error()` 工厂方法，`serialize()` 使用 `\r\n`。
- 待补：`error()` 目前只认 400/404，其余状态码的 `statusText` 应补全（如 405、413、500）。

#### 任务 2.3：完成读写闭环（大部分完成，可靠性待补）

已完成：

- 检查 `accept`/`recv` 返回值；用实际字节数构造字符串。
- `readRequest` 循环读取请求，`sendAll` 循环发送。
- 每条控制流都会 `close(client_fd)`。

待补：

- `recv == 0`（对端关闭）与 `recv < 0`（读取失败）分开处理。
- 恢复 `SO_REUSEADDR`，错误信息带上 `errno` 对应的文本。
- 恢复 `/hello` 路由（200 + `hello`），增加 `/echo` 的最小 body 回显。
- **为 `acceptLoop` 增加 try-catch 异常边界**（见任务 2.4 之后的问题 1、2）。

#### 任务 2.4：按 HTTP 消息边界读取（未完成）

实现一个有大小上限的读取循环：

1. 读取到 `\r\n\r\n`，表示请求头结束。
2. 解析 `Content-Length`。
3. 继续读取，直到请求体达到指定字节数。
4. 超过设定上限时返回 `413 Payload Too Large`。
5. 请求不完整或格式非法时返回 `400 Bad Request`。

当前不实现 chunked encoding；收到 `Transfer-Encoding: chunked` 时返回明确的“不支持”响应即可。

#### 任务 2.5：扩展 `Request` 和 Parser（部分完成）

`Request` 已有 `method / path / version / headers / body / params` 字段，
`Http::parseRequest` 已能解析请求行、headers 和 body。待补：

- 验证请求行恰好包含 method、target、version 三部分；格式错误不能悄悄生成
  半成品 `Request`。
- 去掉 header value 行尾的 `\r`（当前 bug，见进度评估第 5 条）。
- 将 query string 从 path 中拆出保存，不需要做 URL decode（当前 bug，第 4 条）。
- 请求头名称按大小写不敏感处理。

#### 里程碑 2 验收

```bash
curl -i http://127.0.0.1:8080/hello
curl -i http://127.0.0.1:8080/not-found
curl -i -X POST http://127.0.0.1:8080/echo -d 'abc'
```

验收标准：

- `/hello` 返回 `200`，body 为 `hello`。
- 未知路径返回 `404`。
- `/echo` 能原样返回请求体。
- `Content-Length` 与 body 的字节数一致。
- 连续请求 20 次，服务不退出，fd 数量不持续增长。
- **坏请求（非法 JSON、非数字参数）不会杀死进程。**
- Parser 和 Response 至少各有一组不依赖真实网络的单元测试。

---

### 里程碑 3：Router（method 匹配与路径参数已完成，405 待补）

目标：Server 只处理网络和协议，不再用一串 `if (path == ...)` 承载业务。

当前实现评价：

- 已完成：`get()/post()` 注册、`route.method == request.method` 匹配、
  `:id` 路径参数写入 `request.params`、未命中返回 404。
- 待补：区分 `404 Not Found` 与 `405 Method Not Allowed`；拒绝重复注册相同路由；
  Router 兜底 404 应使用统一 JSON 错误格式（当前是纯文本 `"404"`）。
- 学习函数指针、lambda、捕获、`std::function` 的成本和用途。

测试重点：正常匹配、未知路径、方法不匹配、重复路由、路径参数。

---

### 里程碑 4：Controller、JSON 与统一错误（内存版雏形已完成，未验收）

目标：完成一个不接数据库的内存版 REST API。

当前状态：`POST /user`（创建）、`GET /user/:id`（查询）、404 JSON 已可用，分层依赖
注入已建立。与正式验收之间还差：

- **异常保护**：`json::parse`、`stoi`、缺失字段都要有异常边界，坏输入返回
  `400 Bad Request` 而不是杀死进程（当前最优先，见第 6 节任务 A）。
- **输入校验**：name 必填、age 类型/范围校验，校验失败返回 400。
- **统一错误格式**：建议使用 `{"code": ..., "message": ...}`，当前是 `{"error": ...}`，
  二者统一后再写错误响应测试。
- **`GET /users` 列表接口**（Repository 增加 `findAll`，Service/Controller 逐层透传）。
- **正确状态码**：创建成功返回 `201 Created`（当前统一 200）。
- **自动化测试**：Service 业务规则、Controller 状态码映射的纯函数测试。

推荐继续使用 `nlohmann/json`，第三方依赖通过 CMake 明确管理。

职责约束（保持）：

- Router：选择 Handler。
- Controller：读取参数、校验输入、组织 HTTP/JSON 响应。
- Service：实现业务规则，例如用户名不能重复。
- Repository：保存和查询数据；此阶段先用内存实现。

统一错误 JSON 示例：

```json
{
  "code": "invalid_request",
  "message": "username is required"
}
```

学习 HTTP 状态码：`200`、`201`、`400`、`404`、`409`、`500`。

---

### 里程碑 5：SQLite 与 Repository

目标：重启服务后用户数据仍然存在，并理解事务和约束。

任务：

- 使用 SQLite 官方 C API 或成熟 C++ 封装。
- 建立 `users` 表和可重复执行的初始化/迁移逻辑。
- 使用 prepared statement，禁止拼接用户输入生成 SQL。
- 使用数据库唯一约束保证 username 唯一。
- 用 RAII 管理数据库连接、statement 和事务。
- Repository 接口保持稳定，将内存实现替换为 SQLite 实现。

建议字段：`id`、`username`、`password_hash`、`created_at`。不要存储明文密码。

学习点：CRUD、主键、唯一索引、事务、SQL 注入、Repository 模式。

---

### 里程碑 6：注册、登录与认证

目标：实现安全边界清晰的最小认证流程。

任务顺序：

1. 注册时使用 Argon2id 或 bcrypt 等成熟库生成密码哈希。
2. 登录时校验密码，返回认证凭证。
3. 先实现服务端随机 session token，更容易理解撤销和过期。
4. 再将 JWT 作为对比实验，而不是默认认为 JWT 一定更好。
5. 认证中间件读取 `Authorization: Bearer <token>`。
6. 为受保护接口注入当前用户信息。

安全要求：

- 不自行实现哈希、加密或 JWT 签名算法。
- token/密钥不写入代码或提交到 Git。
- token 有过期时间；错误响应不能泄露密码或内部异常。
- 理解 JWT 只是签名 token 格式，并不等于加密，也不天然“无状态且可撤销”。

---

### 里程碑 7：日志、配置与可观测性

任务：

- 从配置文件或环境变量读取端口、数据库路径和密钥。
- 启动时校验配置，失败时给出清晰错误。
- 每个请求生成 request id。
- 记录 method、path、status、耗时；不得记录密码和完整 token。
- 统一捕获 Handler 异常并返回 `500`，同时记录服务端错误。
- 增加 `GET /health` 健康检查。

学习点：访问日志与错误日志、结构化日志、敏感信息处理。

---

### 里程碑 8：并发、超时与优雅退出

在单线程版本测试稳定后，再实现固定大小线程池：

```text
accept thread -> bounded task queue -> worker threads
```

任务：

- 有界任务队列，避免无限占用内存。
- 使用 mutex、condition_variable，理解竞态和临界区。
- 设置读写超时，防止慢客户端永久占用 worker。
- `SIGINT/SIGTERM` 到来后停止接收新连接，处理或终止已有任务，再释放资源。
- 使用并发压测比较单线程与线程池版本，不只看 QPS，也观察错误率和延迟。

注意：一个连接一个线程可以作为过渡实验，但不是最终结构。

---

### 里程碑 9：扩展任务

完成核心路线后再选做：

- 文件上传：学习 `multipart/form-data`、大小限制、文件名与路径安全。
- AI 代理接口：由 `AIService` 调用模型 API，配置超时、重试和流式返回。
- HTTPS：通常由 Nginx/Caddy 终止 TLS，同时理解反向代理和转发头。
- 更完整的 HTTP：keep-alive、chunked encoding；用于学习，不追求替代成熟服务器。
- 使用 Crow/FastAPI/Spring Boot 重写相同 API，对照自己的 Router、Middleware、Service 和 Repository。

---

## 5. 每个里程碑的学习闭环

每完成一个里程碑，记录以下内容：

```text
1. 我实现了什么？
2. 数据从哪里进入，经过哪些对象，从哪里离开？
3. 我遇到了什么 bug，根因是什么？
4. 哪些边界条件有测试？
5. 如果交给成熟框架，它替我做了什么？
6. 当前实现有哪些明确限制？
```

提交代码前检查：

- 能从干净的 `build/` 目录构建。
- 编译无 warning。
- 自动化测试通过。
- `curl` 验收通过。
- 没有提交二进制、数据库文件、密钥或构建产物。
- 一次提交只解决一个清晰问题，提交信息说明“为什么改”。

---

## 6. 当前学习任务（按顺序完成）

每完成一组就单独构建和提交。

### A. 给 Handler 加上异常边界（最优先，实测会崩溃）

1. 在 `Server::acceptLoop` 的处理流程外包一层 `try-catch`：
   - 捕获 `nlohmann::json::parse_error`（或更通用的 `std::exception`），返回
     `400 Bad Request` 而不是让进程退出。
   - 实测验收：`curl -X POST localhost:8080/user -d 'not-json'` 后服务必须继续存活，
     再请求 `GET /user/1` 应正常返回。
2. `getUserById` 的 `std::stoi` 同样要捕获，非数字 id 返回 400。
3. 捕获后先返回 400/500，日志、统一错误格式放到后续任务。
4. 思考题：为什么 `json::parse` 的异常在 main 里没有被任何层拦截？（当前没有任何
   异常边界，异常直接穿过 `main` 触发 `std::terminate`。）

### B. 工程卫生与可重复构建

1. 完成任务 2.1 的收尾项：`CMAKE_CXX_STANDARD_REQUIRED ON`、
   `CMAKE_CXX_EXTENSIONS OFF`、warning 选项只作用于 `minibackend` target。
2. 添加 `.gitignore`（忽略 `build/`、`main` 可执行文件、`.vscode/` 等），停止跟踪
   `MiniBackend/build/` 下的生成文件。
3. 从一个新的空 build 目录执行配置和构建，确认不依赖旧缓存。

验收命令：

```bash
cmake -S MiniBackend -B build
cmake --build build --clean-first
```

验收标准：成功生成 `build/minibackend`，编译输出中没有 warning，`git status` 不再
显示 `build/` 下的文件。

### C. 修复 Parser 与 Router 的已知缺陷

1. `Http::parseRequest`：去掉 header value 行尾的 `\r`（遍历 value 时
   `value.erase(value.find_last_not_of(" \t\r") + 1)`）。
2. `parseRequest`：验证请求行恰好三部分，多/少都返回错误（或抛特定异常由上层映射
   为 400）；格式错误不能悄悄生成半成品 Request。
3. 将 query string 从 path 中拆出（按 `?` 分割存为独立字段），先不做 URL decode。
4. Router：增加 405（方法存在但 method 不匹配）与重复路由注册检查；
   兜底 404 改为统一 JSON 错误格式并带 `Content-Type`。

### D. 恢复 `/hello`、`/echo` 并完成里程碑 2 验收

1. 注册 `GET /hello`（返回 `200` + `hello`）与 `POST /echo`（回显 body）。
2. 给 `readRequest` 加大小上限（超限返回 413）和 chunked 拒绝逻辑（任务 2.4）。
3. 恢复 `SO_REUSEADDR`，系统调用错误信息带上 `errno` 文本；`recv == 0` 与
   `recv < 0` 分开处理。
4. 按“里程碑 2 验收”清单逐条执行，包括连续 20 次请求和坏请求不崩溃。

### E. 补上第一批自动化测试（CTest）

1. 为 `Response::serialize()` 写测试：直接比较完整字符串，包括每一个 `\r\n`；
   覆盖空 body、普通文本 body、自定义 `Content-Type`、自动 `Content-Length`。
2. 为 `Http::parseRequest` 写测试：请求行、headers、body、`\r` 清理、query 拆分、
   格式错误。
3. 为 `Router` 写测试：正常匹配、未知路径、405、重复路由、路径参数。
4. 先用 CTest + 简单测试可执行文件，不必急着引入大型测试框架。

### F. 回到里程碑 4：完善内存版 REST

完成 A～E 后：

1. 输入校验：name 必填、age 为合理整数，失败返回 400。
2. 统一错误 JSON 为 `{"code", "message"}` 结构，并更新 `Response::error()`。
3. 增加 `GET /users` 列表接口（Repository 加 `findAll`）。
4. 创建成功返回 `201 Created`。
5. 为 Service 业务规则和 Controller 状态码映射写测试。
6. 这些任务完成后，再考虑 `/user` 的路由命名是否改为复数 `/users` 以符合 REST 习惯
   （同时保持 `GET /user/:id` 与 `POST /user` 的兼容，或做一次有意的接口切换）。

在这些任务完成之前，不引入 SQLite、JWT、线程池。

---

## 7. 本阶段需要能够回答的问题

完成上述任务时，不仅要让代码运行，还要能用自己的话回答：

1. 为什么 `HTTP/1.1200 OK` 会被判定为非法状态行？
2. 为什么 `Content-Length` 应按字节数而不是“字符个数”的直觉计算？
3. 为什么 `send(fd, data, size, 0)` 成功也可能没有发送完 `size` 个字节？
4. 为什么一次 `recv` 可能只得到半个请求，也可能包含后续数据？
5. 404 与 405 的语义区别是什么？
6. 为什么 build 目录不应该提交到 Git？CMakeCache 为什么不能当源码共享？
7. RAII 如何保证异常发生时 fd 仍能被关闭？
8. 为什么一次非法 JSON 请求就能杀死整个服务进程？成熟框架是怎么防止这种情况的？
9. 为什么 `GET /user/1?x=1` 会返回 404？query string 应该在协议层还是业务层处理？
10. 为什么 header 的 value 会带一个 `\r` 尾巴？请求行解析为什么不会遇到这个问题
    （`operator>>` 会跳过空白，而 `getline` 不会）？
11. 为什么进程崩溃后立刻重启会 `bind failed`？`SO_REUSEADDR` 解决的是什么？
12. 路径参数 `:id` 在 Router 里解析，还是在 Controller 里解析？为什么放在 Router
    更合理？
