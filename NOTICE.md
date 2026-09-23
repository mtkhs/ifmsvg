# 取り込んだライブラリと、その条件

リポジトリの `LICENSE`（MIT）は、本プロジェクトが自分で書いた部分に掛かる。
ビルド時に取り込む以下は、それぞれの条件に従う。

| ライブラリ | 取得元 | 条件 | 組み込み方 |
| --- | --- | --- | --- |
| resvg v0.45.1（C API） | https://github.com/RazrFalcon/resvg | MPL-2.0 | FetchContent + Corrosion で staticlib として静的リンク |
| resvg が依存する Rust crate 群 | Cargo.lock 参照 | 各 crate の LICENSE | 同上（個別の一覧は未作成） |
| zlib | vcpkg | zlib License | 静的リンク（`x64-windows-static-md`） |
| susie.h | TORO's Library http://toro.d.dooo.jp/dlsphapi.html | ヘッダ先頭の表示のとおり | `include/susie.h` |

MPL-2.0 は、実行形式を配布する際に Source Code Form の入手先を示すこと、
resvg 自体を改変した場合はその改変部分を同条件で公開することを求める。
本プロジェクトは resvg を改変せずリンクしているので、入手先（上表の URL）の表示で足りる。
