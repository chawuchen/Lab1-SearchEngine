该文件夹包含了 VS2019 工程

- `SearchEngine.sln` 为解决方案文件

  >  解决方案 `SearchEngine` 包括了三个项目：
  >
  > - `DocumentSearch` 项目：主要用于输入文档 id 查询训练集文档以及 tfidf 值
  > - `SearchEngine` 项目：这个实验的主项目，根据查询搜索文档集返回前 20 个相关的文档
  > - `TestQueryResult` 项目：分析训练集结果的准确率值以及 DCG 等评价指标

- `DocumentSearch/`  DocumentSearch 项目文件夹，包括项目代码

- `SearchEngine/` SearchEngine 项目文件夹，包括项目代码

- `TestQueryResult/`  TestQueryResult 项目文件夹，包括项目代码

- `dict/` 下为需要用到的词典，包含了 cppjieba 库的词典和我们自己的词典