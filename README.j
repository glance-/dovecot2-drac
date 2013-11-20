
DRAC plugin for Dovecot-2.x

必要なもの
-------------
  以下が必要です。

  * dovecot-2.x
  * libdrac

インストール手順
-------------
  1) ビルド
    dovecotのソースディレクトリを DOVDIR で指定して、make を実行します。

    $ make DOVDIR=../dovecot-2.0.13

    LDFLAGSで drac のディレクトリを指定できます。

    $ make DOVDIR=../dovecot-2.0.14 LDFLAGS=-L/usr/local/drac/lib

  2) インストール
    作成された drac_plugin.so を dovecot のライブラリが置かれている
    ディレクトリにコピーします。

    $ su
    # cp -pi drac_plugin.so /usr/lib64/dovecot/modules/

  3) dovecot の設定
    mail_plugins に drac を追加します。
    たとえば、/etc/dovecot/conf.d/10-mail.conf に以下を追加します。

    ===================
    mail_plugins = drac
    ===================

    plugin 内に、dracdserver および dracdtimeout を記述します。
    たとえば、/etc/dovecot/conf.d/90-plugin.conf に以下を追加します。

    ===========================
    plugin {
        ...中略...
        dracdserver = localhost
        dracdtimeout = 60
    }
    ===========================

    dovecot を再起動します。

    # service dovecot restart

ダウンロード
-------------
  http://sourceforge.jp/projects/dovecot2-drac/

連絡先
-------------
  上記sourceforgeサイトを参照してください。
