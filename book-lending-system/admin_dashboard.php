<?php
session_start();
if (!isset($_SESSION['admin_id'])) {
    header("Location: admin_login.php");
    exit();
}
$conn = new mysqli('localhost', 'root', '', 'university_lending_system');
if ($conn->connect_error) die("Connection failed: " . $conn->connect_error);

// Update Borrower
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['update_borrower'])) {
    $stmt = $conn->prepare("UPDATE books SET student_id = ?, student_name = ?, book_name = ?, book_number = ?, date_borrowed = ?, date_returned = ?, status = ? WHERE id = ?");
    $stmt->bind_param("sssssssi", $_POST['student_id'], $_POST['student_name'], $_POST['book_name'], $_POST['book_number'], $_POST['date_borrowed'], $_POST['date_returned'], $_POST['status'], $_POST['book_id']);
    $stmt->execute();
    $status = $_POST['status'];
    $book_id = $_POST['book_id'];
    $stmt = $conn->prepare("UPDATE available_books SET available_copies = available_copies + ? WHERE id = (SELECT available_book_id FROM books WHERE id = ?)");
    $increment = ($status === 'returned') ? 1 : -1;
    $stmt->bind_param("ii", $increment, $book_id);
    $stmt->execute();
    $stmt->close();
}

// Delete Borrower
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['delete_borrower'])) {
    $book_id = $conn->real_escape_string($_POST['book_id']);
    $conn->query("DELETE FROM books WHERE id = $book_id");
}

// Approve Pending Admin
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['approve_admin'])) {
    $pending_id = $conn->real_escape_string($_POST['pending_id']);
    $stmt = $conn->prepare("SELECT username, password FROM pending_admins WHERE id = ?");
    $stmt->bind_param("i", $pending_id);
    $stmt->execute();
    $result = $stmt->get_result();
    if ($result->num_rows === 1) {
        $pending = $result->fetch_assoc();
        $stmt = $conn->prepare("INSERT INTO admins (username, password) VALUES (?, ?)");
        $stmt->bind_param("ss", $pending['username'], $pending['password']);
        if ($stmt->execute()) {
            $conn->query("DELETE FROM pending_admins WHERE id = $pending_id");
        }
    }
    $stmt->close();
}

// Reject Pending Admin
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['reject_admin'])) {
    $pending_id = $conn->real_escape_string($_POST['pending_id']);
    $conn->query("DELETE FROM pending_admins WHERE id = $pending_id");
}

// Approve Pending Student
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['approve_student'])) {
    $pending_id = $conn->real_escape_string($_POST['pending_id']);
    $stmt = $conn->prepare("SELECT student_id, full_name, email, password FROM pending_students WHERE id = ?");
    $stmt->bind_param("i", $pending_id);
    $stmt->execute();
    $result = $stmt->get_result();
    if ($result->num_rows === 1) {
        $pending = $result->fetch_assoc();
        $stmt = $conn->prepare("INSERT INTO students (student_id, full_name, email, password) VALUES (?, ?, ?, ?)");
        $stmt->bind_param("ssss", $pending['student_id'], $pending['full_name'], $pending['email'], $pending['password']);
        if ($stmt->execute()) {
            $conn->query("DELETE FROM pending_students WHERE id = $pending_id");
        }
    }
    $stmt->close();
}

// Reject Pending Student
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['reject_student'])) {
    $pending_id = $conn->real_escape_string($_POST['pending_id']);
    $conn->query("DELETE FROM pending_students WHERE id = $pending_id");
}

// Fetch Borrowers
$search = isset($_GET['search']) ? $conn->real_escape_string($_GET['search']) : '';
$query = "SELECT * FROM books WHERE student_id LIKE '%$search%' OR student_name LIKE '%$search%' OR book_name LIKE '%$search%' OR book_number LIKE '%$search%' ORDER BY id DESC";
$result = $conn->query($query);

// Fetch Pending Admins
$pending_admins_result = $conn->query("SELECT * FROM pending_admins ORDER BY request_date DESC");

// Fetch Pending Students
$pending_students_result = $conn->query("SELECT * FROM pending_students ORDER BY request_date DESC");
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Admin Dashboard</title>
    <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;600&display=swap" rel="stylesheet">
    <link rel="stylesheet" href="common.css">
    <link rel="stylesheet" href="admin.css">
</head>
<body>
    <?php include 'navbar.php'; ?>
    <div class="container">
        <header>
            <h1>Admin Dashboard</h1>
        </header>
        <section class="dashboard-section">
            <div class="section-header">
                <h2>Pending Admin Requests</h2>
            </div>
            <div class="table-wrapper">
                <table class="borrowers-table">
                    <thead>
                        <tr>
                            <th>ID</th>
                            <th>Username</th>
                            <th>Request Date</th>
                            <th>Actions</th>
                        </tr>
                    </thead>
                    <tbody>
                        <?php if ($pending_admins_result && $pending_admins_result->num_rows > 0): ?>
                            <?php while ($row = $pending_admins_result->fetch_assoc()): ?>
                                <tr>
                                    <td><?= $row['id'] ?></td>
                                    <td><?= htmlspecialchars($row['username']) ?></td>
                                    <td><?= $row['request_date'] ?></td>
                                    <td class="actions">
                                        <form method="POST" style="display:inline;" onsubmit="return confirm('Approve this admin?');">
                                            <input type="hidden" name="pending_id" value="<?= $row['id'] ?>">
                                            <button type="submit" name="approve_admin" class="btn-primary">Approve</button>
                                        </form>
                                        <form method="POST" style="display:inline;" onsubmit="return confirm('Reject this request?');">
                                            <input type="hidden" name="pending_id" value="<?= $row['id'] ?>">
                                            <button type="submit" name="reject_admin" class="btn-delete">Reject</button>
                                        </form>
                                    </td>
                                </tr>
                            <?php endwhile; ?>
                        <?php else: ?>
                            <tr><td colspan="4" class="no-data">No pending admin requests.</td></tr>
                        <?php endif; ?>
                    </tbody>
                </table>
            </div>
        </section>
        <section class="dashboard-section">
            <div class="section-header">
                <h2>Pending Student Requests</h2>
            </div>
            <div class="table-wrapper">
                <table class="borrowers-table">
                    <thead>
                        <tr>
                            <th>ID</th>
                            <th>Student ID</th>
                            <th>Full Name</th>
                            <th>Email</th>
                            <th>Request Date</th>
                            <th>Actions</th>
                        </tr>
                    </thead>
                    <tbody>
                        <?php if ($pending_students_result && $pending_students_result->num_rows > 0): ?>
                            <?php while ($row = $pending_students_result->fetch_assoc()): ?>
                                <tr>
                                    <td><?= $row['id'] ?></td>
                                    <td><?= htmlspecialchars($row['student_id']) ?></td>
                                    <td><?= htmlspecialchars($row['full_name']) ?></td>
                                    <td><?= htmlspecialchars($row['email']) ?></td>
                                    <td><?= $row['request_date'] ?></td>
                                    <td class="actions">
                                        <form method="POST" style="display:inline;" onsubmit="return confirm('Approve this student?');">
                                            <input type="hidden" name="pending_id" value="<?= $row['id'] ?>">
                                            <button type="submit" name="approve_student" class="btn-primary">Approve</button>
                                        </form>
                                        <form method="POST" style="display:inline;" onsubmit="return confirm('Reject this request?');">
                                            <input type="hidden" name="pending_id" value="<?= $row['id'] ?>">
                                            <button type="submit" name="reject_student" class="btn-delete">Reject</button>
                                        </form>
                                    </td>
                                </tr>
                            <?php endwhile; ?>
                        <?php else: ?>
                            <tr><td colspan="6" class="no-data">No pending student requests.</td></tr>
                        <?php endif; ?>
                    </tbody>
                </table>
            </div>
        </section>
        <section class="dashboard-section">
            <div class="section-header">
                <h2>Borrowers Management</h2>
                <form method="GET" action="admin_dashboard.php" class="search-form">
                    <input type="text" name="search" placeholder="Search by Student ID, Name, Book..." value="<?= htmlspecialchars($search) ?>">
                    <button type="submit" class="btn-primary">Search</button>
                </form>
            </div>
            <div class="table-wrapper">
                <table class="borrowers-table">
                    <thead>
                        <tr>
                            <th>ID</th>
                            <th>Student ID</th>
                            <th>Name</th>
                            <th>Book Name</th>
                            <th>Book Number</th>
                            <th>Date Borrowed</th>
                            <th>Date Returned</th>
                            <th>Status</th>
                            <th>Actions</th>
                        </tr>
                    </thead>
                    <tbody>
                        <?php if ($result && $result->num_rows > 0): ?>
                            <?php while ($row = $result->fetch_assoc()): ?>
                                <tr>
                                    <td><?= $row['id'] ?></td>
                                    <td><?= htmlspecialchars($row['student_id']) ?></td>
                                    <td><?= htmlspecialchars($row['student_name']) ?></td>
                                    <td><?= htmlspecialchars($row['book_name']) ?></td>
                                    <td><?= htmlspecialchars($row['book_number']) ?></td>
                                    <td><?= $row['date_borrowed'] ?></td>
                                    <td><?= $row['date_returned'] ?: 'N/A' ?></td>
                                    <td class="status <?= strtolower($row['status']) ?>"><?= $row['status'] ?></td>
                                    <td class="actions">
                                        <button class="btn-edit" onclick="openEditModal(<?= $row['id'] ?>, '<?= htmlspecialchars($row['student_id']) ?>', '<?= htmlspecialchars($row['student_name']) ?>', '<?= htmlspecialchars($row['book_name']) ?>', '<?= htmlspecialchars($row['book_number']) ?>', '<?= $row['date_borrowed'] ?>', '<?= $row['date_returned'] ?>', '<?= $row['status'] ?>')">Edit</button>
                                        <form method="POST" class="delete-form" onsubmit="return confirm('Are you sure?');">
                                            <input type="hidden" name="book_id" value="<?= $row['id'] ?>">
                                            <button type="submit" name="delete_borrower" class="btn-delete">Delete</button>
                                        </form>
                                    </td>
                                </tr>
                            <?php endwhile; ?>
                        <?php else: ?>
                            <tr><td colspan="9" class="no-data">No records found.</td></tr>
                        <?php endif; ?>
                    </tbody>
                </table>
            </div>
        </section>
        <div id="editModal" class="modal">
            <div class="modal-content">
                <span class="modal-close" onclick="closeModal()">×</span>
                <h2>Edit Borrower Record</h2>
                <form method="POST" action="admin_dashboard.php" class="form-styled">
                    <input type="hidden" id="editBookId" name="book_id">
                    <div class="form-grid">
                        <div class="form-group">
                            <label>Student ID</label>
                            <input type="text" id="editStudentId" name="student_id" required>
                        </div>
                        <div class="form-group">
                            <label>Student Name</label>
                            <input type="text" id="editStudentName" name="student_name" required>
                        </div>
                        <div class="form-group">
                            <label>Book Name</label>
                            <input type="text" id="editBookName" name="book_name" required>
                        </div>
                        <div class="form-group">
                            <label>Book Number</label>
                            <input type="text" id="editBookNumber" name="book_number" required>
                        </div>
                        <div class="form-group">
                            <label>Date Borrowed</label>
                            <input type="date" id="editDateBorrowed" name="date_borrowed" required>
                        </div>
                        <div class="form-group">
                            <label>Date Returned</label>
                            <input type="date" id="editDateReturned" name="date_returned">
                        </div>
                        <div class="form-group">
                            <label>Status</label>
                            <select id="editStatus" name="status" required>
                                <option value="borrowed">Borrowed</option>
                                <option value="returned">Returned</option>
                            </select>
                        </div>
                    </div>
                    <button type="submit" name="update_borrower" class="btn-primary">Save Changes</button>
                </form>
            </div>
        </div>
    </div>
    <script>
        function openEditModal(id, studentId, studentName, bookName, bookNumber, dateBorrowed, dateReturned, status) {
            document.getElementById('editBookId').value = id;
            document.getElementById('editStudentId').value = studentId;
            document.getElementById('editStudentName').value = studentName;
            document.getElementById('editBookName').value = bookName;
            document.getElementById('editBookNumber').value = bookNumber;
            document.getElementById('editDateBorrowed').value = dateBorrowed;
            document.getElementById('editDateReturned').value = dateReturned || '';
            document.getElementById('editStatus').value = status;
            document.getElementById('editModal').style.display = 'flex';
        }
        function closeModal() {
            document.getElementById('editModal').style.display = 'none';
        }
        window.onclick = function(event) {
            if (event.target === document.getElementById('editModal')) closeModal();
        }
    </script>
</body>
</html>