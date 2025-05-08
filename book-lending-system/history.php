<?php
session_start();
if (!isset($_SESSION['student_id'])) {
    header("Location: student_auth.php");
    exit();
}
$conn = new mysqli('localhost', 'root', '', 'university_lending_system');
if ($conn->connect_error) die("Connection failed: " . $conn->connect_error);
$stmt = $conn->prepare("SELECT * FROM books WHERE student_id = ? ORDER BY date_borrowed DESC");
$stmt->bind_param("s", $_SESSION['student_id']);
$stmt->execute();
$history = $stmt->get_result();
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Borrowing History</title>
    <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;600&display=swap" rel="stylesheet">
    <link rel="stylesheet" href="common.css">
    <link rel="stylesheet" href="history.css">
</head>
<body>
    <?php include 'navbar.php'; ?>
    <div class="container">
        <header>
            <h1>Your Borrowing History</h1>
        </header>
        <section class="dashboard-section">
            <div class="table-wrapper">
                <table class="history-table">
                    <thead>
                        <tr>
                            <th>Book Name</th>
                            <th>Book Number</th>
                            <th>Date Borrowed</th>
                            <th>Date Returned</th>
                            <th>Status</th>
                        </tr>
                    </thead>
                    <tbody>
                        <?php if ($history->num_rows > 0): ?>
                            <?php while ($row = $history->fetch_assoc()): ?>
                                <tr>
                                    <td><?= htmlspecialchars($row['book_name']) ?></td>
                                    <td><?= htmlspecialchars($row['book_number']) ?></td>
                                    <td><?= $row['date_borrowed'] ?></td>
                                    <td><?= $row['date_returned'] ?: 'N/A' ?></td>
                                    <td class="status <?= strtolower($row['status']) ?>"><?= $row['status'] ?></td>
                                </tr>
                            <?php endwhile; ?>
                        <?php else: ?>
                            <tr><td colspan="5" class="no-data">No borrowing history found.</td></tr>
                        <?php endif; ?>
                    </tbody>
                </table>
            </div>
        </section>
    </div>
</body>
</html>
