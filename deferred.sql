CREATE TABLE `deferred` (
  `d_title` varbinary(300) DEFAULT NULL,
  UNIQUE KEY `d_title` (`d_title`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
