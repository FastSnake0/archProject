set -e

mongo <<EOF
db = db.getSiblingDB('arch')
db.createCollection('posts')
db.posts.createIndex({"id": -1}) 
db.posts.createIndex({"user_id": -1}) 
db.createCollection('chat')
db.posts.createIndex({"id": -1}) 
db.posts.createIndex({"user_id": -1}) 
EOF