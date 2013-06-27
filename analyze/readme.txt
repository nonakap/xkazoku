
■ analyzeの使い方

　analyze [-lcdw] SUF [filename]

　　-l 使用コマンド一覧を出します。
　　-c コマンドをチェックします
　　-d スクリプトをダンプします
    -s セーブをチェックします。(２箇所以上にセーブが必要)
　　-w 警告レベルを設定します
　　　-w0 未使用コマンド・未使用パラメータを検出します。
　　　-w1 上に加えて xkazokuで未使用のパラメータを検出します
　　　-w2 上に加えて xkazokuで一部未使用のパラメータを検出します

　　オプションが指定されてない場合は -c と見なします。
　　filenameが指定されていない場合は すべてのスクリプトを検索します。
　　-d指定は filenameが必要です。


　例: analyze -l "c:\program files\do\kazoku\kazoku.suf"
　　　　家族計画のすべてのスクリプトの使用コマンド一覧を表示します

　　　analyze -c "c:\program files\do\kazoku\kazoku.suf"
　　　　家族計画のすべてのスクリプトのコマンドをチェックします

　　　analyze -c -w2 "c:\program files\do\kazoku\kazoku.suf" start
　　　　家族計画の START.ISFのコマンドを警告レベル2でチェックします

　　　analyze -d "c:\program files\do\kazoku\kazoku.suf" start
　　　　家族計画の START.ISFスクリプトをダンプします

　　　analyze -s "c:\program files\do\kazoku\kazoku.suf"
　　　　家族計画の セーブファイルをチェックします

　win32でコマンドプロンプトが解らないって人は analyze.exeに sufファイルを
　ドロップして下さい。





■ analyzeの間違った使い方

　analyze packfile filename
　　packfileから filenameを取り出します

　例: analyze -d "c:\program files\do\kazoku\GGD" title.gg0
　　　　家族計画のGGDファイルから TITLE.GG0を取り出します。

　ですが、win32環境なら susieを使ったほうが早いです、ええ。





■ ゲームの対応方法

　もくじ。
　　1. 準備
　　2. gamedef.txtにエントリを作成します。
　　3. analyzeを実行します。
　　4. セーブデータの互換を合せます。
　　5. 実行と細かい調整
　　6. 付録:設定キー





　1. 準備
　　ゲームをインストールし、２箇所以上にセーブします。
　　(この部分だけはオリジナル版が動作するWindowsが必要です。)





　2. gamedef.txtにエントリを作成します。

　　ゲームをインストールしたフォルダの *.sufを開き、
　　メーカー名とゲームキーをチェックします。


　　Windowsの場合 analyze.exeと同じフォルダに
　　X11の場合 目的の*.sufと同じディレクトリに
　　gamedef.txtというファイル名のテキストファイルを作成し、
　　以下の書式で仮の設定を書きます。
　　(キーには ASCIIしか使わないので ゲーム名はShiftJISとEUCのどちらも通る筈)


　　gamedef.txt ---------------------------------------------------

　　[(ゲーム名)]
　　company = (メーカー名)
　　key = (ゲームキー名)
　　ver = 
　　type = 

    ---------------------------------------------------------------

　例: MOEKKO.SUF
　　　> COMPANY=ZACKZACK
　　　> KEY=MOEKKO

　　　だった場合

　　　　[萌えッ娘ナース]
　　　　company = ZACKZACK
　　　　key = MOEKKO
　　　　ver = 
　　　　type = 

　　　と、なります。





　3. analyzeを実行します。

　　目的の*.sufファイルを指定して analyzeを実行します。

　　USAGE: analyze [complete SUF path]

　　エラーが表示された　以下のルールに従い gamedef.txtの設定を変更して
　　再度実行します。
　　バージョン名については 本ドキュメントの最後の章を参考にして下さい。
　　通常、発売日の近い物を選択しておくと良いでしょう。


　　エラー説明
　　　warning error : xkazokuで強行します。
　　　　　　　　　　　テキスト、フェードアウトの時間指定等の効果が失われます。
　　　　　　　　　　　一部コマンドで入力で正しい情報を返せない為 継続不能に
　　　　　　　　　　　なる場合もあります。

　　　severe error  : xkazokuで処理出来ません。
　　　　　　　　　　　処理時点でエラーが出て終了してしまいます。

　　対策
　　　error: wrong parameter
　　　　指定バージョンで コマンドのパラメータ数が足りません。
　　　　EXEVERの指定を下げて下さい。

　　　error: unsupport command xx
　　　　指定バージョンで 対応していないコマンドを使用しました。
　　　　EXEVERの指定を上げて下さい。
　　　　それでも解決しない場合は報告をお願いします。

　　　warning: wrong delimiter
　　　　指定バージョンで コマンドに処理しないパラメータが存在します。
　　　　EXEVERの指定を上げて下さい。
　　　　※ 単にゴミが入ってるだけの場合もあります。

　　　warning: unsupport textcmd xx
　　　　テキストに半角メッセージを使用のゲームの場合に発生します。
　　　　GAME_TEXTASCIIを付けて下さい。


　　例: 
　　　ver = EXEVER_PLANET
　　　type = 

　　　> D:\xkazoku>analyze e:\moekko\moekko.suf
　　　> COMPANY: ZACKZACK
　　　　　　：
　　　> TURNCHCK.ISF
　　　> warning: wrong delimiter
　　　> 0014e: 55 - d0 07 00 00 00 00 00 01    (※星ぷらとパラメータ数が違う
　　　>
　　　>  3114 warning errors
　　　>     1  severe errors


　　　ver = EXEVER_DM
　　　type = 

　　　> D:\xkazoku>analyze e:\moekko\moekko.suf
　　　> COMPANY: ZACKZACK
　　　> TITLE: uGeaabu?aiu[aX
　　　> KEY: MOEKKO
　　　> EV001.ISF
　　　> warning: unsupport textcmd 83          (※半角メッセージを使用)
　　　> 01e91: 2b - 00 ff 83 76 83 89 5c 00 83 68 3f 1a 54 3d 90 6c
　　　>           - 8a d4 36 3e 39 16 37 16 17 22 37 24 03 00 00
　　　　　　：
　　　> TURNCHCK.ISF
　　　>    29 warning errors
　　　>     1  severe errors


　　　ver = EXEVER_DM
　　　type = GAME_TEXTASCII

　　　> D:\xkazoku>analyze e:\moekko\moekko.suf
　　　> COMPANY: ZACKZACK
　　　　　：
　　　> LOADSYS.ISF
　　　> warning: unsupport ef 6                (萌えッ娘ナース拡張コマンド)
　　　> 00089: ef - 06 10 00 00 00 00 00 00 00 00 00 00 00 10 00 00
　　　>           - 00 00 00 00 00
　　　　　：
　　　> TURNCHCK.ISF
　　　>     1 warning errors
　　　>     0  severe errors


　　　ver = EXEVER_MOEKKO
　　　type = GAME_TEXTASCII

　　　> D:\xkazoku>analyze e:\moekko\moekko.suf
　　　> COMPANY: ZACKZACK
　　　　　：
　　　> TURNCHCK.ISF
　　　>     0 warning errors
　　　>     0  severe errors





　4. セーブデータの互換を合せます。

　　-s オプション付きで analyzeを実行し、フラグを設定します。
　　EXEVERの設定は使用していませんので 3.よりも先に設定しても良いでしょう。

　　例:
　　　> D:\xkazoku>analyze -s e:\moekko\moekko.suf
　　　> COMPANY: ZACKZACK
　　　> TITLE: uGeaabu?aiu[aX
　　　> KEY: MOEKKO
　　　> use extend save commands
　　　> HLN: value flag 600 (2404bytes)
　　　> FLN: bit flag 701 (88bytes)
　　　> EXA: bit flag 501 (64bytes)
　　　> EXA: value flag 501 (2008bytes)
　　　> KIDFN: kid flag 30000
　　　> load e:/moekko/MOEKKO.sav
　　　> savefile version.2 (later EXEVER_DM)                             (※)
　　　> header files 53 / size 6480bytes
　　　> game saves 50, set configure GAME_SAVEMAX50                      (※)
　　　> analyze game save 1 / filepos 8560 / size 23412bytes
　　　> date: 2003/ 4/26 02:11:12
　　　> use bmp: set configure GAME_SAVEGRPH                             (※)
　　　> bmp size 96x72 - free format
　　　> use comment: set configure GAME_SAVECOM - type:old or save ver2  (※)

　　　※を追加すると (EXEVER_DM < EXEVER_MOEKKO なので EXEVERは変更なし)

　　　ver = EXEVER_MOEKKO
　　　type = GAME_TEXTASCII + GAME_SAVEMAX50 + GAME_TEXTASCII +
　　　　　　 GAME_SAVEGRPH + GAME_SAVECOM

　　　と なります。





　5. 実行と細かい調整

　　オリジナルと比較して 以下の設定をつけ加えます。

　　　GAME_VOICE      - オプションに 「音声」の選択あり

　　　GAME_VOICEONLY  - オプションに 「音声のみ」の選択あり (GAME_VOICEも設定)

　　　GAME_SELECTEX   - 選択中に 指定範囲以外にマウスを移動した場合にも
　　　　　　　　　　　　フォーカスを表示させます。
　　　　　　　　　　　　(家族計画タイプ)

　　　GAME_TEXTLAYER  - エフェクト中にテキストレイヤーも描画します。
                        (旧パープル, 青い涙タイプ)

　　　GAME_CMDWINNOBG - 選択肢のバックグラウンドを表示させない。
                        (青い涙タイプ)

　　　これらは 単に表示を一部変更させるための物で 設定しなかったとしても
　　　ゲーム基本部分には関係ありません。

　　　GAME_VOICE と GAME_VOICEONLYは 単に EXEのデフォルト値の指定で、
　　　実際には スクリプト上で変更できる筈なのですが、意図的に変更してる
　　　ゲームは見たことはありません…。(全てデフォルト指定のまんま)


　　最後に作成した gamedef.txtを
　　　WinCEの場合 akira.exeと同じフォルダに
　　　MacOSの場合 xkazokuと同じフォルダに
　　　X11の場合 目的の*.sufと同じディレクトリに
　　コピーすると パッチが有効になります。





　6. 付録:設定キー

　　ver = EXEVER_KANA         99-06-16 加奈
          EXEVER_MYU          99-07-16 Purple
          EXEVER_TSUKU        00-02-15 尽くしてあげちゃう
          EXEVER_AMEIRO       00-04-18 あめいろの季節
          EXEVER_TEA2         00-08-10 せ・ん・せ・い２
          EXEVER_PLANET       00-11-27 星空ぷらねっと
          EXEVER_PLANDVD      01-01-31 星空ぷらねっとDVD
          EXEVER_NURSE        01-06-01 プライベートナース
          EXEVER_KONYA        01-07-15 私に今夜☆会いに来て
          EXEVER_VECHO        01-08-29 ヴェルベットエコー
          EXEVER_CRES         01-09-11 クレシェンド
          EXEVER_KAZOKU       01-10-24 家族計画
          EXEVER_BLEED        02-05-10 出血簿
          EXEVER_OSHIETE      02-08-19 教えてあげちゃう
          EXEVER_SISKON       02-09-02 しすこん
          EXEVER_KONYA2       02-10-03 私に今夜☆会いに来て２
          EXEVER_KAZOKUK      02-11-27 家族計画 絆箱
          EXEVER_HEART        03-01-13 はぁとdeルームメイト
          EXEVER_SHISYO       03-01-16 司書さんといっしょ☆
          EXEVER_DM           03-03-11 ドーターメーカー
          EXEVER_MOEKKO       03-04-04 萌えッ娘ナース
          EXEVER_AOI          03-05-07 青い涙
          EXEVER_KAMOOK       03-05-30 家族計画～絆本～
          EXEVER_RESTORE      03-06-11 れすとあ


　　type = (+ 区切りで複数指定可能)
          GAME_VOICE          ヴォイスのサポート
          GAME_VOICEONLY      ヴォイスのみのサポート
          GAME_HAVEALPHA      テキスト窓可変通過処理サポート
          GAME_TEXTASCII      半角テキストを許す
          GAME_DRS            DRS system
          GAME_SVGA           800x600
          GAME_NOKID          既読フラグなし
          GAME_SAVEMAX9       セーブ数 9
          GAME_SAVEMAX27      セーブ数 27
          GAME_SAVEMAX30      セーブ数 30
          GAME_SAVEMAX50      セーブ数 50
          GAME_SAVEGRPH       グラフィックを含む
          GAME_SAVECOM        コメントを含む
          GAME_SAVESYS        システム領域有り
          GAME_SELECTEX       選択フォーカスを殺さない
          GAME_TEXTLAYER      テキストレイヤー分離
          GAME_CMDWINNOBG     選択肢バックグラウンドなし

　　type指定については 複数行で行なえます。
　　２行目以降は + 最初のオプションの前にを付けて下さい。

　　　type = GAME_TEXTASCII + GAME_SAVEMAX50 + GAME_TEXTASCII
　　　type = + GAME_SAVEGRPH + GAME_SAVECOM

