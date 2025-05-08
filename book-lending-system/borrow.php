<?php
session_start();
if (!isset($_SESSION['student_id'])) {
    header("Location: student_auth.php");
    exit();
}
$conn = new mysqli('localhost', 'root', '', 'university_lending_system');
if ($conn->connect_error) die("Connection failed: " . $conn->connect_error);

// Check if student has unreturned books
$student_id = $_SESSION['student_id'];
$check_stmt = $conn->prepare("SELECT COUNT(*) FROM books WHERE student_id = ? AND status = 'borrowed' AND (date_returned IS NULL OR date_returned = '')");
$check_stmt->bind_param("s", $student_id);
$check_stmt->execute();
$check_stmt->bind_result($unreturned_count);
$check_stmt->fetch();
$check_stmt->close();

if ($unreturned_count > 0) {
    $error = "You cannot borrow another book until all borrowed books are returned.";
} elseif ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $stmt = $conn->prepare("INSERT INTO books (student_id, student_name, book_name, book_number, date_borrowed, available_book_id, status) VALUES (?, ?, ?, ?, ?, ?, 'borrowed')");
    $stmt->bind_param("sssssi", $_POST['student_id'], $_POST['student_name'], $_POST['book_name'], $_POST['book_number'], $_POST['date_borrowed'], $_POST['available_book_id']);
    if ($stmt->execute()) {
        $conn->query("UPDATE available_books SET available_copies = available_copies - 1 WHERE id = " . $conn->real_escape_string($_POST['available_book_id']));
        header("Location: browse.php?success=Borrowed+successfully");
        exit();
    } else {
        $error = "Failed to borrow book.";
    }
    $stmt->close();
}

$book_id = isset($_GET['book_id']) ? $_GET['book_id'] : '';
$book_name = isset($_GET['book_name']) ? urldecode($_GET['book_name']) : '';
$book_number = isset($_GET['book_number']) ? urldecode($_GET['book_number']) : '';
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Borrow Book</title>
    <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;600&display=swap" rel="stylesheet">
    <link rel="stylesheet" href="common.css">
</head>
<body>
    <?php include 'navbar.php'; ?>
    <div class="container">
        <section class="borrow-section">
            <header>
                <h1><span class="header-icon">📚</span> Borrow a Book</h1>
            </header>
            <form action="borrow.php" method="POST" class="form-styled">
                <input type="hidden" name="available_book_id" value="<?= htmlspecialchars($book_id) ?>">
                <div class="form-grid">
                    <div class="form-group">
                        <label for="student_id">Student ID</label>
                        <input type="text" id="student_id" name="student_id" value="<?= $_SESSION['student_id'] ?>" readonly>
                    </div>
                    <div class="form-group">
                        <label for="student_name">Student Name</label>
                        <input type="text" id="student_name" name="student_name" placeholder="Enter your Full Name" required>
                    </div>
                    <div class="form-group">
                        <label for="book_name">Book Name</label>
                        <input type="text" id="book_name" name="book_name" value="<?= htmlspecialchars($book_name) ?>" readonly>
                    </div>
                    <div class="form-group">
                        <label for="book_number">Book Number</label>
                        <input type="text" id="book_number" name="book_number" value="<?= htmlspecialchars($book_number) ?>" readonly>
                    </div>
                    <div class="form-group full-width">
                        <label for="date_borrowed">Date Borrowed</label>
                        <input type="date" id="date_borrowed" name="date_borrowed" value="<?= date('Y-m-d') ?>" required>
                    </div>
                </div>
                <button type="submit" class="btn-primary" <?= isset($error) && $unreturned_count > 0 ? 'disabled' : '' ?>>Borrow Book</button>
                <?php if (isset($error)): ?>
                    <p class="error-message"><?= htmlspecialchars($error) ?></p>
                <?php endif; ?>
            </form>
        </section>
    </div>
    <style>
        .borrow-section {
            background: #fff;
            padding: 40px;
            border-radius: 12px;
            box-shadow: 0 5px 20px rgba(0, 0, 0, 0.05);
            max-width: 600px;
            margin: 0 auto;
        }
        header h1 {
            font-size: 36px;
            font-weight: 600;
            color: #2b6cb0;
            text-align: center;
            margin-bottom: 20px;
        }
        .header-icon {
            font-size: 40px;
            vertical-align: middle;
        }
        .form-styled {
            display: flex;
            flex-direction: column;
            gap: 20px;
        }
        .form-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
        }
        .form-group label {
            font-size: 14px;
            font-weight: 500;
            color: #718096;
            margin-bottom: 8px;
            display: block;
        }
        .form-group input {
            width: 100%;
            padding: 12px 15px;
            border: 1px solid #e2e8f0;
            border-radius: 8px;
            font-size: 16px;
            background: #f7fafc;
        }
        .form-group input:focus {
            border-color: #2b6cb0;
            outline: none;
        }
        .form-group input[readonly] {
            background: #edf2f7;
            color: #718096;
        }
        .full-width {
            grid-column: 1 / -1;
        }
        .error-message {
            color: #e53e3e;
            font-size: 14px;
            text-align: center;
            margin-top: 10px;
        }
        .btn-primary:disabled {
            background: #a0aec0;
            cursor: not-allowed;
        }
    </style>
</body>
</html>
