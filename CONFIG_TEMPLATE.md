# Configuration Template - Poultrix Edge AI
# Copy this file to config.h and update with your actual credentials

## WiFi Configuration
```cpp
#define WIFI_SSID "your_wifi_network_name"
#define WIFI_PASSWORD "your_wifi_password"
```

## Firebase Configuration
```cpp
#define DATABASE_URL "https://your-project-id-default-rtdb.firebaseio.com"
#define DATABASE_SECRET "your_firebase_secret_key"
```

## Security Notes
- Never commit actual credentials to public repositories
- Use environment variables in production
- Consider using Firebase App Check for additional security
- Rotate secrets regularly

## For Competition Demo
- Update credentials in main.cpp before uploading
- Update firebaseConfig in poultrix-dashboard.js
- Test connection before presentation

## Firebase Setup Steps
1. Create Firebase project at https://console.firebase.google.com
2. Go to Project Settings > Service Accounts
3. Generate new private key or use legacy tokens
4. Enable Realtime Database
5. Set database rules for read/write access during development:
   ```json
   {
     "rules": {
       ".read": true,
       ".write": true
     }
   }
   ```
   
## Production Security
For production deployment, implement proper authentication and security rules.