-- TinyWebServer 数据库初始化脚本
-- docker-compose 启动时自动执行

CREATE DATABASE IF NOT EXISTS qgydb
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE qgydb;

CREATE TABLE IF NOT EXISTS user (
    id       INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(64)  NOT NULL UNIQUE,
    passwd   VARCHAR(128) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 测试账号（开发用，生产环境删除）
INSERT IGNORE INTO user (username, passwd) VALUES
    ('admin', 'admin123'),
    ('test',  'test123');
