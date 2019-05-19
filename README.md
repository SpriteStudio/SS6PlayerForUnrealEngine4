### SS6Player for Unreal Engine 4

- ドキュメント  
https://github.com/SpriteStudio/SS6PlayerForUnrealEngine4/wiki

- チュートリアル  
http://www.webtech.co.jp/help/ja/spritestudio/guide/output6/unrealengine4/

- BluePrintリファレンス  
https://github.com/SpriteStudio/SS6PlayerForUnrealEngine4/wiki/BluePrintリファレンス

- Component/Widget、プロパティリファレンス ?
http://www.webtech.co.jp/help/ja/spritestudio/guide/output6/unrealengine4/component/

- 制限事項  
https://github.com/SpriteStudio/SS6PlayerForUnrealEngine4/wiki/TIPS-制限事項


##### 対応UE4バージョン
UE4.22

※ 旧バージョンのUE4で使用したい場合は、該当のTagから取得して下さい

※ v1.4.0_UE4.22 での仕様変更/追加機能について
- 一部アセット名を変更しました。旧バージョンから更新する場合は、上書きではなく、一度削除してから再度コピーして下さい。
- Componentで描画モード「加算」「減算」に対応しました。但し、SpriteStudio上でのブレンド方法とは厳密には異なります。
- UMGで描画モード「加算」に対応しました。但し、SpriteStudio上でのブレンド方法とは厳密には異なります。
- Component/Widgetの描画モードに「Masked」を追加しました。アルファ値を反映せずMakedマテリアルで描画します。
- Component/Widgetにパーツのマテリアルを動的に差し替える機能を追加しました。(AddMaterialReplacement等)
- ProjectSettingsから、描画モード，アルファブレンドモード，パーツカラーモードに応じてデフォルトマテリアルを上書き出来るようになりました。

※ v1.4.1_UE4.22 での追加機能について
- メッシュパーツを使用したアニメーション再生時のメモリリークを修正しました。
- Component/Widgetに、ブレンドモード単位でマテリアルを動的に差し替える機能を追加しました。
- Component/Widgetに、実際に描画に使用されているMaterialInstanceDynamicのリストを取得できるプロパティを追加しました。

