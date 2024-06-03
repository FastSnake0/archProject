set -e

mongo <<EOF
db = db.getSiblingDB('arch')
db.createCollection('posts')
db.posts.createIndex({"id": -1}) 
db.posts.createIndex({"user_id": -1}) 
db.createCollection('chat')
db.chat.createIndex({"id": -1}) 
db.chat.createIndex({"from_id": -1}) 
db.chat.createIndex({"to_id": -1}) 
EOF