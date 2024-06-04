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
db.message.insertOne({"id": 2, "from_id": 1, "to_id": 3, "text": "1 -> 3", "timestamp": "2024-06-01T18:28:48Z",})
db.message.insertOne({"id": 3, "from_id": 1, "to_id": 7, "text": "1 -> 7", "timestamp": "2024-06-02T18:38:48Z",})
db.message.insertOne({"id": 4, "from_id": 1, "to_id": 12, "text": "1 -> 12", "timestamp": "2024-06-03T18:48:48Z",})
db.message.insertOne({"id": 5, "from_id": 1, "to_id": 17, "text": "1 -> 17", "timestamp": "2024-06-03T18:58:48Z",})

db.message.insertOne({"id": 6, "from_id": 3, "to_id": 1, "text": "3 -> 1", "timestamp": "2024-06-01T18:28:49Z",})
db.message.insertOne({"id": 7, "from_id": 7, "to_id": 1, "text": "7 -> 1", "timestamp": "2024-06-02T18:38:49Z",})
db.message.insertOne({"id": 8, "from_id": 12, "to_id": 1, "text": "12 -> 1", "timestamp": "2024-06-03T18:48:49Z",})
db.message.insertOne({"id": 9, "from_id": 17, "to_id": 1, "text": "17 -> 1", "timestamp": "2024-06-03T18:58:49Z",})

db.post.insertOne({"id": 2, "text": "test2", "timestamp": "2024-06-01T14:55:19Z", "title": "TEST2", "user_id": 1})
db.post.insertOne({"id": 3, "text": "test3", "timestamp": "2024-06-01T15:57:19Z", "title": "TEST3", "user_id": 4})
db.post.insertOne({"id": 4, "text": "test4", "timestamp": "2024-06-01T15:58:19Z", "title": "TEST4", "user_id": 8})
db.post.insertOne({"id": 5, "text": "test5", "timestamp": "2024-06-02T11:57:19Z", "title": "TEST5", "user_id": 12})
db.post.insertOne({"id": 6, "text": "test6", "timestamp": "2024-06-02T12:57:19Z", "title": "TEST6", "user_id": 16})
db.post.insertOne({"id": 7, "text": "test7", "timestamp": "2024-06-02T13:57:19Z", "title": "TEST7", "user_id": 20})
db.post.insertOne({"id": 8, "text": "test8", "timestamp": "2024-06-03T10:57:19Z", "title": "TEST8", "user_id": 24})
db.post.insertOne({"id": 9, "text": "test9", "timestamp": "2024-06-03T11:57:19Z", "title": "TEST9", "user_id": 28})
db.post.insertOne({"id": 10, "text": "test10", "timestamp": "2024-06-03T11:57:19Z", "title": "TEST10", "user_id": 32})
EOF