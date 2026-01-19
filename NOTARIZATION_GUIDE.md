# Apple 公証（Notarization）とコード署名の手順

macOSでプラグインを販売・配布する場合、Appleのセキュリティ要件を満たすために**コード署名（Code Signing）**と**公証（Notarization）**が必要です。これを行わないと、ユーザーがインストールしようとした際に「開発元が未確認のため開けません」という警告が表示されます。

## 必要なもの
- **Apple Developer Program への登録**（年間 99ドル）
- **Xcode** がインストールされたMac
- **App-Specific Password**（Apple IDの管理ページで生成）

---

## 手順 1: コード署名 (Code Signing)

ビルドされたプラグインとインストーラーに署名します。正規の証明書（Developer ID Application / Installer）を使用してください。

### プラグイン本体への署名
```bash
codesign --force --options runtime --timestamp --sign "Developer ID Application: Your Name (TeamID)" "EA VT-2W.vst3"
codesign --force --options runtime --timestamp --sign "Developer ID Application: Your Name (TeamID)" "EA VT-2W.component"
```

### インストーラー (.pkg) への署名
```bash
productsign --sign "Developer ID Installer: Your Name (TeamID)" "EA_VT-2W_Installer_macOS_unsigned.pkg" "EA_VT-2W_Installer_macOS.pkg"
```

---

## 手順 2: 公証 (Notarization)

Appleのサーバーにインストーラーをアップロードしてスキャンを受けます。

```bash
xcrun notarytool submit EA_VT-2W_Installer_macOS.pkg --apple-id "your-apple-id@email.com" --password "app-specific-password" --team-id "YOUR_TEAM_ID" --wait
```

スキャンが成功（Accepted）したら、次のステップへ進みます。

---

## 手順 3: ステープル (Stapling)

公証済みの証明書をインストーラー自体に埋め込みます。これにより、オフライン環境でも公証済みであることが認識されます。

```bash
xcrun stapler staple EA_VT-2W_Installer_macOS.pkg
```

---

## 注意事項
- **Team ID** は Apple Developer Account のページで確認できます。
- **Notarization** はインターネット接続が必要です。
- 署名を忘れると公証は必ず失敗します。
