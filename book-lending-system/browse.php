<?php
session_start();
$conn = new mysqli('localhost', 'root', '', 'university_lending_system');
if ($conn->connect_error) die("Connection failed: " . $conn->connect_error);
$books = $conn->query("SELECT * FROM available_books WHERE available_copies > 0");
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Browse Books</title>
    <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;600&display=swap" rel="stylesheet">
    <link rel="stylesheet" href="common.css">
    <link rel="stylesheet" href="browse.css">
</head>
<body>
    <?php include 'navbar.php'; ?>
    <div class="container">
        <header>
            <h1>Browse Our Collection</h1>
        </header>
        <section class="book-section">
            <div class="section-header">
                <h2>All Books</h2>
                <div class="search-bar">
                    <input type="text" id="searchInput" placeholder="Search books...">
                    <button onclick="searchBooks()">Search</button>
                </div>
            </div>
            <div class="book-grid" id="bookGrid">
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
                                <?php if (isset($_SESSION['student_id'])): ?>
                                    <a href="borrow.php?book_id=<?= urlencode($book['id']) ?>&book_name=<?= urlencode($book['book_name']) ?>&book_number=<?= urlencode($book['book_number']) ?>" class="btn-borrow">Borrow</a>
                                <?php else: ?>
                                    <a href="studentauth.php" class="btn-borrow">Sign In to Borrow</a>
                                <?php endif; ?>
                            </div>
                        </div>
                    <?php endwhile; ?>
                <?php else: ?>
                    <p class="no-data">No books available right now.</p>
                <?php endif; ?>
            </div>
        </section>
    </div>
    <script>
        function searchBooks() {
            const searchValue = document.getElementById('searchInput').value.toLowerCase();
            const bookCards = document.querySelectorAll('.book-card');
            bookCards.forEach(card => {
                const title = card.querySelector('.book-title').textContent.toLowerCase();
                const number = card.querySelector('.book-meta').textContent.toLowerCase();
                card.style.display = (title.includes(searchValue) || number.includes(searchValue)) ? 'block' : 'none';
            });
        }
    </script>
</body>
</html>
