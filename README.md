# ifmsvg.sph - SVG/SVGZ Susie Plug-in

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: Windows](https://img.shields.io/badge/Platform-Windows-blue.svg)](https://www.microsoft.com/windows)

- SVG および gzip 圧縮された SVGZ 形式の画像を Susie 対応画像ビューアで表示するための 64bit プラグインです
- レンダリングには [resvg](https://github.com/RazrFalcon/resvg) を使用しています
- あふｗで動作確認しています

## 特徴

- SVG / SVGZ 形式 (*.svg, *.svgz) の画像ファイルの読み込み
- 設定ファイルで解像度 (DPI)・最大出力サイズ・背景・フォントを指定可能である
- Windows システムフォントを読み込むのでテキストを含む SVG も表示できる

## ダウンロード

ビルド済みの `ifmsvg.sph` は [Releases](https://github.com/mtkhs/ifmsvg/releases) から取得できます。

## インストール

- 動作環境: Windows 10/11 (64bit) と 64bit 対応 Susie プラグインホスト（あふｗ 等）

1. `ifmsvg.sph` を Susie プラグインフォルダに置いてください
2. 必要に応じて `ifmsvg.ini`（後述）を同じフォルダに置いてください
3. ホストアプリケーション側でプラグインを有効化してください

## 設定ファイル（任意）

`ifmsvg.sph` と同じフォルダに `ifmsvg.ini` があれば読み込みます（無ければデフォルト値で動きます）。

```ini
[render]
; 解像度（DPI）。範囲外は無視してデフォルトに戻す。既定: 150、許容: 30〜1200
dpi=150

; 最大出力サイズ（各辺のピクセル数）。0 で無制限。既定: 8192
max_size=8192

; 背景処理（colored または transparent）。既定: colored
background=colored

; 背景色（RRGGBB 16進）。background=colored のとき有効。既定: FFFFFF
background_color=FFFFFF

[font]
; Windows システムフォントを読み込むか。既定: true
load_system_fonts=true

; フォントが解決できなかった場合のフォールバックファミリー。既定: Arial
default_family=Arial

; 追加で読み込むフォントディレクトリ（セミコロン区切り）。既定: 空
extra_dirs=
```

## ビルド

### 必要な環境

| ツール | バージョン |
|--------|-----------|
| Visual Studio 2022 | Build Tools または Community 以上 |
| CMake | 4.0 以上 |
| vcpkg | 最新版（`VCPKG_ROOT` 環境変数を設定）|
| Rust (rustup) | stable x86_64-pc-windows-msvc |

### ビルド手順

```powershell
# Release ビルド（初回は FetchContent で Corrosion + resvg を取得するため時間がかかります）
.\build.ps1

# Debug ビルド
.\build.ps1 -Configuration Debug

# 強制リビルド
.\build.ps1 -Rebuild
```

成功すると `build\Release\ifmsvg.sph` が生成されます。

### テスト実行

```powershell
ctest --test-dir build -C Release --output-on-failure
```

## エクスポート関数

| 関数 | 説明 |
|------|------|
| `GetPluginInfo(W)` | プラグイン情報の取得（`"00IN"`, `"*.svg;*.svgz"` 等） |
| `IsSupported(W)` | ファイルが対応フォーマットか判定 |
| `GetPictureInfo(W)` | 画像サイズ・DPI 等のメタ情報取得 |
| `GetPicture(W)` | 画像を HLOCAL DIB（BGR24）として取得 |
| `GetPreview(W)` | 未実装（`NOTSUPPORT` を返す） |
| `ConfigurationDlg` | 未実装（`NOTSUPPORT` を返す） |

## 📚 参考資料

### 開発に使用したソフトウェア・参考にした情報
- **[Susie 32bit / 64bit Plug-in の仕様(2025-8-10版) - TORO's Library](http://toro.d.dooo.jp/dlsphapi.html)**: susie.h を拝借
- **[runspx](https://github.com/toroidj/runspx)**: APIの動作確認用
- **[resvg](https://github.com/RazrFalcon/resvg)**: SVG のレンダリングエンジン

## ライセンス

MIT License — Copyright (c) mtkhs

### 依存ライブラリの著作権表示

本プラグイン（ifmsvg.sph）は以下のライブラリを静的リンクしています。

**resvg** — MIT / Apache-2.0  
Copyright 2017 the Resvg Authors  
https://github.com/RazrFalcon/resvg

**tiny-skia** — BSD-3-Clause  
Copyright (c) 2011 Google Inc. All rights reserved.  
Copyright (c) 2020 Yevhenii Reizner. All rights reserved.  
https://github.com/linebender/tiny-skia

**zlib** — zlib License  
Copyright (C) 1995-2022 Jean-loup Gailly and Mark Adler  
https://zlib.net/
