<?php
session_start();
$conn = new mysqli('localhost', 'root', '', 'university_lending_system');
if ($conn->connect_error) die("Connection failed: " . $conn->connect_error);

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['login'])) {
    $username = $conn->real_escape_string($_POST['username']);
    $password = $_POST['password'];
    $stmt = $conn->prepare("SELECT * FROM admins WHERE username = ?");
    $stmt->bind_param("s", $username);
    $stmt->execute();
    $result = $stmt->get_result();
    if ($result->num_rows === 1) {
        $admin = $result->fetch_assoc();
        if (password_verify($password, $admin['password'])) {
            $_SESSION['admin_id'] = $admin['id'];
            header("Location: admin_dashboard.php");
            exit();
        } else {
            $login_error = "Invalid username or password";
        }
    } else {
        $login_error = "Invalid username or password";
    }
    $stmt->close();
}

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['create_admin'])) {
    $username = $conn->real_escape_string($_POST['new_username']);
    $password = password_hash($_POST['new_password'], PASSWORD_DEFAULT);

    // Check if username already exists in admins or pending_admins
    $check_stmt = $conn->prepare("SELECT username FROM admins WHERE username = ? UNION SELECT username FROM pending_admins WHERE username = ?");
    $check_stmt->bind_param("ss", $username, $username);
    $check_stmt->execute();
    $result = $check_stmt->get_result();

    if ($result->num_rows > 0) {
        $create_error = "Username already exists or is pending approval.";
    } else {
        $stmt = $conn->prepare("INSERT INTO pending_admins (username, password) VALUES (?, ?)");
        $stmt->bind_param("ss", $username, $password);
        if ($stmt->execute()) {
            $create_success = "Admin account request submitted. Awaiting approval.";
        } else {
            $create_error = "Failed to submit request. Try again.";
        }
        $stmt->close();
    }
    $check_stmt->close();
}
$conn->close();
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Admin Login</title>
    <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;600&display=swap" rel="stylesheet">
    <link rel="stylesheet" href="common.css">
    <link rel="stylesheet" href="auth.css">
</head>
<body>
    <?php include 'navbar.php'; ?>
    <div class="auth-container">
        <h1 id="formTitle">Admin Login</h1>
        <form method="POST" class="form-styled" id="loginForm">
            <input type="hidden" name="login" value="1">
            <div class="form-group">
                <label for="username">Username</label>
                <input type="text" id="username" name="username" required>
            </div>
            <div class="form-group">
                <label for="password">Password</label>
                <input type="password" id="password" name="password" required>
            </div>
            <button type="submit" class="btn-primary">Login</button>
            <?php if (isset($login_error)): ?>
                <p class="error-message"><?= htmlspecialchars($login_error) ?></p>
            <?php endif; ?>
        </form>
        <p class="form-toggle" onclick="toggleForms()">Request Admin Account</p>
        <form method="POST" class="form-styled form-hidden" id="createForm">
            <input type="hidden" name="create_admin" value="1">
            <div class="form-group">
                <label for="new_username">New Username</label>
                <input type="text" id="new_username" name="new_username" required>
            </div>
            <div class="form-group">
                <label for="new_password">New Password</label>
                <input type="password" id="new_password" name="new_password" required>
            </div>
            <button type="submit" class="btn-primary">Submit Request</button>
            <?php if (isset($create_error)): ?>
                <p class="error-message"><?= htmlspecialchars($create_error) ?></p>
            <?php endif; ?>
            <?php if (isset($create_success)): ?>
                <p style="color: #38a169; font-size: 14px; margin-top: 10px;"><?= htmlspecialchars($create_success) ?></p>
            <?php endif; ?>
        </form>
        <p class="form-toggle form-hidden" onclick="toggleForms()" id="backToLogin">Back to Login</p>
    </div>
    <script>
        function toggleForms() {
            const loginForm = document.getElementById('loginForm');
            const createForm = document.getElementById('createForm');
            const backToLogin = document.getElementById('backToLogin');
            const formTitle = document.getElementById('formTitle');
            if (loginForm.classList.contains('form-hidden')) {
                loginForm.classList.remove('form-hidden');
                createForm.classList.add('form-hidden');
                backToLogin.classList.add('form-hidden');
                formTitle.textContent = "Admin Login";
            } else {
                loginForm.classList.add('form-hidden');
                createForm.classList.remove('form-hidden');
                backToLogin.classList.remove('form-hidden');
                formTitle.textContent = "Request Admin Account";
            }
        }
    </script>
</body>
</html>