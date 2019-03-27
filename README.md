# explord
Expérimentation Libre par Ordinateur est un projet d'EXAO Libre. Signifie également Expérimentation Pédagogique, Libre, Ouverte, Radio et Digitale

Une sonde à dioxygène sans fil est codée avec une ESP32 modèle TTGO T1 et un capteur LOX-02. Ainsi qu'un capteur DHT22 avec une autre carte ESP32 permet de fournir un capteur de température et d'humidité.

L'interface graphique utilise la Web Bluetooth API implémentée sur Google Chrome uniquement ce qui permet d'utiliser cette interface codée en HTML, CSS et Javascript sous Linux, Android (à partir de Android 7.0), MacOS (non testé) et Windows 10.

Pour utiliser ce systéme EXAO sans fil fonctionnant avec le BLE, il faut :
- une carte Bluetooth version 4.0 minimum pour utiliser le Bluetooth Low Energy (BLE) et 4.1 minimum pour pouvoir connecter plusieurs clients BLE sur un même capteur qui a le rôle de serveur BLE (multi-connect);
- Google Chrome (version 70 minimum pour Windows 10).
Il est également possible d'utiliser la carte connectée à un ordinateur à l'aide d'un port USB pour Windows 7 par exemple ou pour un ordinateur ne possédant pas la carte Bluetooth adapté. Dans ce cas d'autres clients peuvent se connecter également si ils ont les caractéristiques minimales requises.
