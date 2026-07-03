-- TinyWebServer local MySQL initialization script.
-- docker compose mounts this file into /docker-entrypoint-initdb.d/.

CREATE DATABASE IF NOT EXISTS qgydb
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE qgydb;

-- Legacy login/register table used by the original CGI-style flow.
CREATE TABLE IF NOT EXISTS user (
    id         INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    username   VARCHAR(64)  NOT NULL UNIQUE,
    passwd     VARCHAR(128) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS books (
    id          INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    title       VARCHAR(128) NOT NULL,
    author      VARCHAR(128) NOT NULL,
    price_cents INT UNSIGNED NOT NULL,
    stock       INT UNSIGNED NOT NULL DEFAULT 0,
    status      VARCHAR(32)  NOT NULL DEFAULT 'active',
    created_at  TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at  TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_books_status (status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS inventory (
    book_id    INT UNSIGNED PRIMARY KEY,
    available  INT UNSIGNED NOT NULL DEFAULT 0,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT fk_inventory_book
        FOREIGN KEY (book_id) REFERENCES books(id)
        ON DELETE RESTRICT
        ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS inventory_reservations (
    reservation_id VARCHAR(96) PRIMARY KEY,
    status         VARCHAR(32) NOT NULL DEFAULT 'reserved',
    created_at     TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at     TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_inventory_reservations_status (status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS inventory_reservation_items (
    id             INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    reservation_id VARCHAR(96) NOT NULL,
    book_id        INT UNSIGNED NOT NULL,
    quantity       INT UNSIGNED NOT NULL,
    created_at     TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY uniq_inventory_reservation_item (reservation_id, book_id),
    INDEX idx_inventory_reservation_items_book_id (book_id),
    CONSTRAINT fk_inventory_reservation_items_reservation
        FOREIGN KEY (reservation_id) REFERENCES inventory_reservations(reservation_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    CONSTRAINT fk_inventory_reservation_items_book
        FOREIGN KEY (book_id) REFERENCES books(id)
        ON DELETE RESTRICT
        ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS orders (
    id          INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    user_id     INT UNSIGNED NOT NULL,
    status      VARCHAR(32)  NOT NULL DEFAULT 'created',
    total_cents INT UNSIGNED NOT NULL DEFAULT 0,
    created_at  TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at  TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_orders_user_id (user_id),
    INDEX idx_orders_status (status),
    CONSTRAINT fk_orders_user
        FOREIGN KEY (user_id) REFERENCES user(id)
        ON DELETE RESTRICT
        ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS order_items (
    id               INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    order_id         INT UNSIGNED NOT NULL,
    book_id          INT UNSIGNED NOT NULL,
    quantity         INT UNSIGNED NOT NULL,
    unit_price_cents INT UNSIGNED NOT NULL,
    line_total_cents INT UNSIGNED NOT NULL,
    created_at       TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_order_items_order_id (order_id),
    INDEX idx_order_items_book_id (book_id),
    CONSTRAINT fk_order_items_order
        FOREIGN KEY (order_id) REFERENCES orders(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    CONSTRAINT fk_order_items_book
        FOREIGN KEY (book_id) REFERENCES books(id)
        ON DELETE RESTRICT
        ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Development seed accounts. Remove or replace in production.
INSERT IGNORE INTO user (id, username, passwd) VALUES
    (1,  'admin',   'admin123'),
    (2,  'test',    'test123'),
    (3,  'alice',   'alice123'),
    (4,  'bob',     'bob123'),
    (5,  'charlie', 'charlie123'),
    (6,  'diana',   'diana123'),
    (7,  'eve',     'eve123'),
    (8,  'frank',   'frank123'),
    (9,  'grace',   'grace123'),
    (10, 'henry',   'henry123');

INSERT INTO books (id, title, author, price_cents, stock, status) VALUES
    (1,  'C++ Primer',                          'Stanley Lippman',           9800,  12, 'active'),
    (2,  'Unix Network Programming',            'W. Richard Stevens',       12800,   4, 'active'),
    (3,  'Effective Modern C++',                'Scott Meyers',              8900,   8, 'active'),
    (4,  'Design Patterns',                     'Gang of Four',             10500,   6, 'active'),
    (5,  'Clean Code',                          'Robert C. Martin',          7600,  15, 'active'),
    (6,  'The Pragmatic Programmer',            'Hunt & Thomas',             8200,  10, 'active'),
    (7,  'Introduction to Algorithms',          'Cormen et al.',            15800,   3, 'active'),
    (8,  'Computer Networking',                 'Kurose & Ross',            11200,   7, 'active'),
    (9,  'Database System Concepts',            'Silberschatz',             13500,   5, 'active'),
    (10, 'Operating System Concepts',           'Silberschatz',             14200,   4, 'active'),
    (11, '三体',                                '刘慈欣',                    4500,   0, 'inactive'),
    (12, '红楼梦',                              '曹雪芹',                    6800,   0, 'active'),
    (13, 'Deep Learning',                       'Goodfellow et al.',         19800,   2, 'active'),
    (14, 'TCP/IP Illustrated, Volume 1',        'W. Richard Stevens',       16800,   3, 'active'),
    (15, 'Kubernetes in Action',                'Marko Luksa',               9200,   9, 'active'),
    (16, 'Code Complete',                       'Steve McConnell',           9900,  11, 'active'),
    (17, 'Refactoring',                         'Martin Fowler',             8400,   8, 'active'),
    (18, 'The Mythical Man-Month',                'Frederick Brooks',        6200,   6, 'active'),
    (19, 'The Go Programming Language',           'Donovan & Kernighan',    11800,   6, 'active'),
    (20, 'The Rust Programming Language',         'Klabnik & Nichols',      10200,   5, 'active'),
    (21, 'Fluent Python',                         'Luciano Ramalho',         9400,   8, 'active'),
    (22, 'Java Concurrency in Practice',          'Brian Goetz',            12500,   4, 'active'),
    (23, 'Spring in Action',                      'Craig Walls',             8800,  10, 'active'),
    (24, 'Domain-Driven Design',                  'Eric Evans',             15600,   3, 'active'),
    (25, 'Designing Data-Intensive Applications', 'Martin Kleppmann',       14200,   7, 'active'),
    (26, 'Site Reliability Engineering',          'Google SRE Team',        13600,   5, 'active'),
    (27, 'Prometheus Up and Running',             'Brian Brazil',            7800,   6, 'active'),
    (28, 'Docker Deep Dive',                      'Nigel Poulton',           6900,  12, 'active'),
    (29, 'Python Crash Course',                   'Eric Matthes',            7200,  14, 'active'),
    (30, '活着',                                  '余华',                    3800,  20, 'active'),
    (31, '围城',                                  '钱钟书',                  4200,   8, 'active'),
    (32, '百年孤独',                              '加西亚·马尔克斯',         5600,   6, 'active'),
    (33, '人类简史',                              '尤瓦尔·赫拉利',           6800,   9, 'active'),
    (34, '黑客与画家',                            'Paul Graham',             5200,   7, 'active'),
    (35, 'SICP',                                  'Abelson & Sussman',      11800,   2, 'active'),
    (36, 'Computer Architecture',                 'Hennessy & Patterson',   17800,   2, 'active'),
    (37, 'Compilers: Principles and Practice',    'Aho, Lam, Sethi',        16200,   1, 'active'),
    (38, 'Pattern Recognition and ML',            'Christopher Bishop',     18800,   2, 'active'),
    (39, 'Hands-On Machine Learning',             'Aurelien Geron',          9600,  11, 'active'),
    (40, 'Introduction to Linear Algebra',        'Gilbert Strang',         11400,   4, 'active'),
    (41, 'Linear Algebra Done Right',             'Sheldon Axler',           8600,   5, 'active'),
    (42, 'Calculus',                              'James Stewart',          15200,   3, 'active'),
    (43, '明朝那些事儿',                          '当年明月',                9800,  15, 'active'),
    (44, '枪炮、病菌与钢铁',                      'Jared Diamond',           7200,   6, 'active'),
    (45, 'Zero to One',                           'Peter Thiel',             5800,   8, 'active'),
    (46, 'The Lean Startup',                      'Eric Ries',               6400,   9, 'active'),
    (47, 'The C Programming Language',            'Kernighan & Ritchie',     7400,  10, 'active'),
    (48, 'Head First Design Patterns',            'Freeman & Robson',        8100,  13, 'active'),
    (49, 'sed and awk',                           'Dougherty & Robbins',     5400,   4, 'active'),
    (50, 'Programming Pearls',                    'Jon Bentley',             6700,   7, 'active')
ON DUPLICATE KEY UPDATE
    title = VALUES(title),
    author = VALUES(author),
    price_cents = VALUES(price_cents),
    stock = VALUES(stock),
    status = VALUES(status);

INSERT INTO inventory (book_id, available) VALUES
    (1,  12),
    (2,   4),
    (3,   8),
    (4,   6),
    (5,  15),
    (6,  10),
    (7,   3),
    (8,   7),
    (9,   5),
    (10,  4),
    (11,  0),
    (12,  0),
    (13,  2),
    (14,  3),
    (15,  9),
    (16, 11),
    (17,  8),
    (18,  6),
    (19,  6),
    (20,  5),
    (21,  8),
    (22,  4),
    (23, 10),
    (24,  3),
    (25,  7),
    (26,  5),
    (27,  6),
    (28, 12),
    (29, 14),
    (30, 20),
    (31,  8),
    (32,  6),
    (33,  9),
    (34,  7),
    (35,  2),
    (36,  2),
    (37,  1),
    (38,  2),
    (39, 11),
    (40,  4),
    (41,  5),
    (42,  3),
    (43, 15),
    (44,  6),
    (45,  8),
    (46,  9),
    (47, 10),
    (48, 13),
    (49,  4),
    (50,  7)
ON DUPLICATE KEY UPDATE
    available = VALUES(available);

INSERT INTO orders (id, user_id, status, total_cents) VALUES
    (1,  2, 'created',   19600),
    (2,  3, 'created',   24100),
    (3,  4, 'created',   15800),
    (4,  2, 'created',   29600),
    (5,  5, 'cancelled', 10500),
    (6,  6, 'created',   29600),
    (7,  3, 'created',   24600),
    (8,  7, 'created',   19800),
    (9,  8, 'created',   38000),
    (10, 9, 'created',   22700)
ON DUPLICATE KEY UPDATE
    user_id = VALUES(user_id),
    status = VALUES(status),
    total_cents = VALUES(total_cents);

INSERT INTO order_items (id, order_id, book_id, quantity, unit_price_cents, line_total_cents) VALUES
    (1,  1,  1,  2,  9800, 19600),
    (2,  2,  3,  1,  8900,  8900),
    (3,  2,  5,  2,  7600, 15200),
    (4,  3,  7,  1, 15800, 15800),
    (5,  4,  8,  1, 11200, 11200),
    (6,  4, 15,  2,  9200, 18400),
    (7,  5,  4,  1, 10500, 10500),
    (8,  6,  2,  1, 12800, 12800),
    (9,  6, 14,  1, 16800, 16800),
    (10, 7,  6,  3,  8200, 24600),
    (11, 8, 13,  1, 19800, 19800),
    (12, 9,  9,  1, 13500, 13500),
    (13, 9, 16,  1,  9900,  9900),
    (14, 9, 17,  1,  8400,  8400),
    (15, 9, 18,  1,  6200,  6200),
    (16, 10, 5,  1,  7600,  7600),
    (17, 10, 3,  1,  8900,  8900),
    (18, 10, 18, 1,  6200,  6200)
ON DUPLICATE KEY UPDATE
    order_id = VALUES(order_id),
    book_id = VALUES(book_id),
    quantity = VALUES(quantity),
    unit_price_cents = VALUES(unit_price_cents),
    line_total_cents = VALUES(line_total_cents);
