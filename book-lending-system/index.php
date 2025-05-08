<?php
session_start();
$conn = new mysqli('localhost', 'root', '', 'university_lending_system');
if ($conn->connect_error) die("Connection failed: " . $conn->connect_error);
$books = $conn->query("SELECT * FROM available_books WHERE available_copies > 0 LIMIT 3");
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Book Lending System</title>
    <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;600&display=swap" rel="stylesheet">
    <link rel="stylesheet" href="common.css">
    <link rel="stylesheet" href="index.css">
</head>
<body>
    <?php include 'navbar.php'; ?>
    <div class="hero">
        <div class="hero-content">
            <h1>Welcome to University Book Lending</h1>
            <p>Borrow your favorite books with ease and explore our collection!</p>
            <a href="browse.php" class="hero-btn">Explore Now</a>
        </div>
    </div>
    <div class="container">
        <section class="book-section">
            <div class="section-header">
                <h2>Featured Books</h2>
                <p>Check out some of our available titles</p>
            </div>
            <div class="book-grid">
                <?php if ($books->num_rows > 0): ?>
                    <?php while ($book = $books->fetch_assoc()): ?>
                        <?php $image_path = "images/" . htmlspecialchars($book['book_number']) . ".jpg"; ?>
                        <div class="book-card">
                            <div class="book-image">
                                <?php if (file_exists($image_path)): ?>
                                    <img src="<?= $image_path ?>" alt="<?= htmlspecialchars($book['book_name']) ?>" loading="lazy">
                                <?php else: ?>
                                    <div class="image-placeholder"><span class="book-icon">📖</span></div>
                                <?php endif; ?>
                            </div>
                            <div class="book-info">
                                <h3 class="book-title"><?= htmlspecialchars($book['book_name']) ?></h3>
                                <p class="book-meta">Book #<?= htmlspecialchars($book['book_number']) ?></p>
                                <p class="availability"><span class="availability-label">Available:</span>
                                    <span class="availability-count <?= $book['available_copies'] > 0 ? 'in-stock' : 'out-of-stock' ?>">
                                        <?= $book['available_copies'] ?>
                                    </span>
                                </p>
                            </div>
                        </div>
                    <?php endwhile; ?>
                <?php else: ?>
                    <div class="no-books"><span class="empty-icon">📚</span><p>No books available right now.</p></div>
                <?php endif; ?>
            </div>
            <a href="browse.php" class="explore-btn">Browse All Books</a>
        </section>
    </div>
</body>
</html>

