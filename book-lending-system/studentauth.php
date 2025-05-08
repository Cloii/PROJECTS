<?php
session_start();
$conn = new mysqli('localhost', 'root', '', 'university_lending_system');
if ($conn->connect_error) die("Connection failed: " . $conn->connect_error);

// Login
$login_error = '';
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['login'])) {
    $student_id = $conn->real_escape_string($_POST['student_id']);
    $password = $_POST['password'];
    $stmt = $conn->prepare("SELECT * FROM students WHERE student_id = ?");
    $stmt->bind_param("s", $student_id);
    $stmt->execute();
    $result = $stmt->get_result();
    if ($result->num_rows === 1) {
        $student = $result->fetch_assoc();
        if (password_verify($password, $student['password'])) {
            $_SESSION['student_id'] = $student['student_id'];
            header("Location: index.php");
            exit();
        } else {
            $login_error = "Invalid Student ID or Password";
        }
    } else {
        $login_error = "Invalid Student ID or Password";
    }
    $stmt->close();
}

// Signup (Request Account)
$signup_error = '';
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['signup'])) {
    $student_id = $conn->real_escape_string($_POST['student_id']);
    $full_name = $conn->real_escape_string($_POST['full_name']);
    $email = $conn->real_escape_string($_POST['email']);
    $password = password_hash($_POST['password'], PASSWORD_DEFAULT);

    // Check if student_id or email already exists in students or pending_students
    $check_stmt = $conn->prepare("SELECT student_id, email FROM students WHERE student_id = ? OR email = ? UNION SELECT student_id, email FROM pending_students WHERE student_id = ? OR email = ?");
    $check_stmt->bind_param("ssss", $student_id, $email, $student_id, $email);
    $check_stmt->execute();
    $result = $check_stmt->get_result();
    if ($result->num_rows > 0) {
        $row = $result->fetch_assoc();
        $signup_error = $row['student_id'] === $student_id ? "Student ID already exists or is pending approval." : "Email already registered or is pending approval.";
    } else {
        $stmt = $conn->prepare("INSERT INTO pending_students (student_id, full_name, email, password) VALUES (?, ?, ?, ?)");
        $stmt->bind_param("ssss", $student_id, $full_name, $email, $password);
        if ($stmt->execute()) {
            $signup_success = "Account request submitted. Awaiting admin approval.";
        } else {
            $signup_error = "Failed to submit request. Try again.";
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
    <title>Student Authentication</title>
    <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;600&display=swap" rel="stylesheet">
    <link rel="stylesheet" href="common.css">
    <link rel="stylesheet" href="auth.css">
</head>
<body>
    <?php include 'navbar.php'; ?>
    <div class="auth-container">
        <h1 id="formTitle">Student Login</h1>
        <form method="POST" class="form-styled" id="loginForm">
            <input type="hidden" name="login" value="1">
            <div class="form-group">
                <label for="student_id">Student ID</label>
                <input type="text" id="student_id" name="student_id" required>
            </div>
            <div class="form-group">
                <label for="password">Password</label>
                <input type="password" id="password" name="password" required>
            </div>
            <button type="submit" class="btn-primary">Login</button>
            <?php if ($login_error): ?>
                <p class="error-message"><?= htmlspecialchars($login_error) ?></p>
            <?php endif; ?>
        </form>
        <p class="form-toggle" onclick="toggleForms()">Need an account? Request Signup</p>
        <form method="POST" class="form-styled form-hidden" id="signupForm">
            <input type="hidden" name="signup" value="1">
            <div class="form-group">
                <label for="signup_student_id">Student ID</label>
                <input type="text" id="signup_student_id" name="student_id" required>
            </div>
            <div class="form-group">
                <label for="full_name">Full Name</label>
                <input type="text" id="full_name" name="full_name" required>
            </div>
            <div class="form-group">
                <label for="email">Email</label>
                <input type="email" id="email" name="email" required>
            </div>
            <div class="form-group">
                <label for="signup_password">Password</label>
                <input type="password" id="signup_password" name="password" required>
            </div>
            <button type="submit" class="btn-primary">Submit Request</button>
            <?php if ($signup_error): ?>
                <p class="error-message"><?= htmlspecialchars($signup_error) ?></p>
            <?php endif; ?>
            <?php if (isset($signup_success)): ?>
                <p style="color: #38a169; font-size: 14px; margin-top: 10px;"><?= htmlspecialchars($signup_success) ?></p>
            <?php endif; ?>
        </form>
        <p class="form-toggle form-hidden" onclick="toggleForms()" id="backToLogin">Back to Login</p>
    </div>
    <script>
        function toggleForms() {
            const loginForm = document.getElementById('loginForm');
            const signupForm = document.getElementById('signupForm');
            const backToLogin = document.getElementById('backToLogin');
            const formTitle = document.getElementById('formTitle');
            if (loginForm.classList.contains('form-hidden')) {
                loginForm.classList.remove('form-hidden');
                signupForm.classList.add('form-hidden');
                backToLogin.classList.add('form-hidden');
                formTitle.textContent = "Student Login";
            } else {
                loginForm.classList.add('form-hidden');
                signupForm.classList.remove('form-hidden');
                backToLogin.classList.remove('form-hidden');
                formTitle.textContent = "Student Signup Request";
            }
        }
    </script>
</body>
</html>