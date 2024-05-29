-- Создание таблицы users
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    first_name VARCHAR(256) NOT NULL,
    last_name VARCHAR(256) NOT NULL,
    login VARCHAR(256) NOT NULL UNIQUE,
    password VARCHAR(256) NOT NULL,
    email VARCHAR(256) NULL UNIQUE,
    title VARCHAR(1024) NULL
);