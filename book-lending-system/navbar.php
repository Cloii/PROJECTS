<?php
session_start();
$conn = new mysqli('localhost', 'root', '', 'university_lending_system');
if ($conn->connect_error) die("Connection failed: " . $conn->connect_error);
$student_name = '';
if (isset($_SESSION['student_id'])) {
    $stmt = $conn->prepare("SELECT full_name FROM students WHERE student_id = ?");
    $stmt->bind_param("s", $_SESSION['student_id']);
    $stmt->execute();
    $result = $stmt->get_result();
    if ($row = $result->fetch_assoc()) $student_name = $row['full_name'];
    $stmt->close();
}
?>
<nav class="navbar">
    <div class="nav-container">
        <div class="nav-brand">
            <span class="nav-icon">📚</span>
            <span class="nav-title">Book Lending</span>
        </div>
        <ul class="nav-menu">
            <li><a href="index.php" class="<?= basename($_SERVER['PHP_SELF']) == 'index.php' ? 'active' : '' ?>">Home</a></li>
            <li><a href="browse.php" class="<?= basename($_SERVER['PHP_SELF']) == 'browse.php' ? 'active' : '' ?>">Browse Books</a></li>
            <?php if (isset($_SESSION['student_id'])): ?>
                <li class="dropdown">
                    <span>Signed in as: <?= htmlspecialchars($student_name) ?> (<?= $_SESSION['student_id'] ?>)</span>
                    <div class="dropdown-content">
                        <a href="history.php">Borrowing History</a>
                        <a href="logout.php">Logout</a>
                    </div>
                </li>
            <?php elseif (isset($_SESSION['admin_id'])): ?>
                <li class="dropdown">
                    <span>Signed in as: Admin</span>
                    <div class="dropdown-content">
                        <a href="admin_dashboard.php">Dashboard</a>
                        <a href="logout.php">Logout</a>
                    </div>
                </li>
            <?php else: ?>
                <li><a href="studentauth.php">Student Login/Signup</a></li>
                <li><a href="admin_login.php">Admin Login</a></li>
            <?php endif; ?>
        </ul>
    </div>
</nav>
<?php $conn->close(); ?>