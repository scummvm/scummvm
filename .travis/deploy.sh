#!/bin/sh
if [ -z "$DEPLOY_KEY" ] || [ -z "$DEPLOY_HOST" ]; then
	echo 'Not uploading artifacts; one of DEPLOY_{KEY,HOST} is not set'
	exit 0
fi

base64 -d </dev/null >/dev/null 2>&1 && BASE64="base64 -d" || BASE64="base64 -D"
echo "$DEPLOY_KEY" | $BASE64 > /tmp/upload.key
chmod 600 /tmp/upload.key

export SSH_ASKPASS=/bin/echo
echo "put artifacts/* $DEPLOY_PATH" > /tmp/sftp.batch
sftp -b /tmp/sftp.batch -r -i /tmp/upload.key \
     -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no \
     "$DEPLOY_HOST"
