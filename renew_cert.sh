iocage exec nextcloud "certbot renew && service nginx reload"
cat /dev/null | openssl s_client -showcerts -servername hughscloud.com -connect hughscloud.com:443 2>/dev/null | openssl x509 -inform pem -noout -text
