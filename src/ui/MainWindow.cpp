#include "ui/MainWindow.h"
#include "ui/VaultCreationDialog.h"
#include "ui/VaultViewDialog.h"
#include "core/SecureVault.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QCloseEvent>

namespace crimson {
namespace ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , vault_(std::make_unique<crimson::core::SecureVault>())
    , auto_lock_timer_(new QTimer(this))
    , central_widget_(nullptr)
    , main_layout_(nullptr)
    , welcome_widget_(nullptr)
    , vault_widget_(nullptr) {
    
    setupUI();
    setupMenuBar();
    setupStatusBar();
    
    // Setup auto-lock timer
    auto_lock_timer_->setInterval(1000); // Check every second
    connect(auto_lock_timer_, &QTimer::timeout, this, &MainWindow::checkAutoLock);
    
    showWelcomeScreen();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    setWindowTitle("Crimson Lock");
    setMinimumSize(800, 600);
    resize(1000, 700);
    
    central_widget_ = new QWidget;
    setCentralWidget(central_widget_);
    
    main_layout_ = new QVBoxLayout(central_widget_);
    main_layout_->setContentsMargins(20, 20, 20, 20);
    main_layout_->setSpacing(20);
    
    setupWelcomeScreen();
    setupVaultScreen();
}

void MainWindow::setupWelcomeScreen() {
    welcome_widget_ = new QWidget;
    welcome_layout_ = new QVBoxLayout(welcome_widget_);
    welcome_layout_->setAlignment(Qt::AlignCenter);
    welcome_layout_->setSpacing(30);
    
    // Title
    title_label_ = new QLabel("Crimson Lock");
    title_label_->setAlignment(Qt::AlignCenter);
    title_label_->setStyleSheet("font-size: 36px; font-weight: bold; color: #2E86AB;");
    
    // Subtitle
    subtitle_label_ = new QLabel(
        "Military-grade password management with hardware-based security\n\n"
        "• Hardware random number generation\n"
        "• AES-256 encryption\n"
        "• Secure memory management\n"
        "• Offline operation"
    );
    subtitle_label_->setAlignment(Qt::AlignCenter);
    subtitle_label_->setStyleSheet("font-size: 16px; color: #555; line-height: 1.5;");
    
    // Buttons
    create_vault_btn_ = new QPushButton("Create New Vault");
    create_vault_btn_->setMinimumHeight(50);
    create_vault_btn_->setStyleSheet(
        "QPushButton { font-size: 16px; background-color: #2E86AB; color: white; border: none; border-radius: 8px; padding: 15px; }"
        "QPushButton:hover { background-color: #1E5F7A; }"
        "QPushButton:pressed { background-color: #0E3F5A; }"
    );
    
    open_vault_btn_ = new QPushButton("Open Existing Vault");
    open_vault_btn_->setMinimumHeight(50);
    open_vault_btn_->setStyleSheet(
        "QPushButton { font-size: 16px; background-color: #A23B72; color: white; border: none; border-radius: 8px; padding: 15px; }"
        "QPushButton:hover { background-color: #7A2B52; }"
        "QPushButton:pressed { background-color: #5A1B32; }"
    );
    
    connect(create_vault_btn_, &QPushButton::clicked, this, &MainWindow::onCreateVault);
    connect(open_vault_btn_, &QPushButton::clicked, this, &MainWindow::onOpenVault);
    
    welcome_layout_->addWidget(title_label_);
    welcome_layout_->addWidget(subtitle_label_);
    welcome_layout_->addSpacing(30);
    welcome_layout_->addWidget(create_vault_btn_);
    welcome_layout_->addWidget(open_vault_btn_);
    welcome_layout_->addStretch();
    
    main_layout_->addWidget(welcome_widget_);
}

void MainWindow::setupVaultScreen() {
    vault_widget_ = new QWidget;
    vault_layout_ = new QVBoxLayout(vault_widget_);
    vault_layout_->setSpacing(20);
    
    // Status label
    vault_status_label_ = new QLabel("Vault Unlocked");
    vault_status_label_->setAlignment(Qt::AlignCenter);
    vault_status_label_->setStyleSheet("font-size: 24px; font-weight: bold; color: #2E86AB; padding: 20px;");
    
    // Action buttons
    vault_actions_layout_ = new QHBoxLayout;
    vault_actions_layout_->setSpacing(15);
    
    create_entry_btn_ = new QPushButton("Create New Entry");
    create_entry_btn_->setMinimumHeight(50);
    create_entry_btn_->setStyleSheet(
        "QPushButton { font-size: 16px; background-color: #F18F01; color: white; border: none; border-radius: 8px; padding: 15px; }"
        "QPushButton:hover { background-color: #D17001; }"
        "QPushButton:pressed { background-color: #B15001; }"
    );
    
    view_vault_btn_ = new QPushButton("View Vault");
    view_vault_btn_->setMinimumHeight(50);
    view_vault_btn_->setStyleSheet(
        "QPushButton { font-size: 16px; background-color: #C73E1D; color: white; border: none; border-radius: 8px; padding: 15px; }"
        "QPushButton:hover { background-color: #A73E1D; }"
        "QPushButton:pressed { background-color: #871E0D; }"
    );
    
    lock_vault_btn_ = new QPushButton("Lock Vault");
    lock_vault_btn_->setMinimumHeight(50);
    lock_vault_btn_->setStyleSheet(
        "QPushButton { font-size: 16px; background-color: #666; color: white; border: none; border-radius: 8px; padding: 15px; }"
        "QPushButton:hover { background-color: #555; }"
        "QPushButton:pressed { background-color: #333; }"
    );
    
    connect(create_entry_btn_, &QPushButton::clicked, this, &MainWindow::onCreateEntry);
    connect(view_vault_btn_, &QPushButton::clicked, this, &MainWindow::onViewVault);
    connect(lock_vault_btn_, &QPushButton::clicked, this, &MainWindow::onLockVault);
    
    vault_actions_layout_->addWidget(create_entry_btn_);
    vault_actions_layout_->addWidget(view_vault_btn_);
    vault_actions_layout_->addWidget(lock_vault_btn_);
    
    // Security status
    security_status_label_ = new QLabel;
    security_status_label_->setAlignment(Qt::AlignCenter);
    security_status_label_->setStyleSheet("color: #666; font-style: italic; padding: 10px;");
    
    vault_layout_->addWidget(vault_status_label_);
    vault_layout_->addLayout(vault_actions_layout_);
    vault_layout_->addWidget(security_status_label_);
    vault_layout_->addStretch();
    
    vault_widget_->hide();
    main_layout_->addWidget(vault_widget_);
}

void MainWindow::setupMenuBar() {
    auto* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&New Vault...", this, &MainWindow::onCreateVault, QKeySequence::New);
    fileMenu->addAction("&Open Vault...", this, &MainWindow::onOpenVault, QKeySequence::Open);
    fileMenu->addSeparator();
    fileMenu->addAction("&Lock Vault", this, &MainWindow::onLockVault, QKeySequence("Ctrl+L"));
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", this, &QWidget::close, QKeySequence::Quit);
    
    auto* vaultMenu = menuBar()->addMenu("&Vault");
    vaultMenu->addAction("&Create Entry...", this, &MainWindow::onCreateEntry, QKeySequence("Ctrl+N"));
    vaultMenu->addAction("&View Entries...", this, &MainWindow::onViewVault, QKeySequence("Ctrl+V"));
    
    auto* toolsMenu = menuBar()->addMenu("&Tools");
    toolsMenu->addAction("&Settings...", this, &MainWindow::onSettings);
    
    auto* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About...", this, &MainWindow::onAbout);
}

void MainWindow::setupStatusBar() {
    statusBar()->showMessage("Ready");
}

void MainWindow::showWelcomeScreen() {
    vault_widget_->hide();
    welcome_widget_->show();
    auto_lock_timer_->stop();
    updateSecurityStatus();
}

void MainWindow::showVaultScreen() {
    welcome_widget_->hide();
    vault_widget_->show();
    auto_lock_timer_->start();
    updateSecurityStatus();
}

void MainWindow::updateSecurityStatus() {
    if (vault_->isOpen()) {
        auto stats = vault_->getStats();
        security_status_label_->setText(
            QString("Vault contains %1 entries • Auto-lock enabled • Hardware RNG active")
            .arg(stats.entryCount)
        );
        statusBar()->showMessage("Vault unlocked - Auto-lock active");
    } else {
        statusBar()->showMessage("No vault open");
    }
}

void MainWindow::onCreateVault() {
    QString masterPassword = getSecurePasswordInput(
        "Create New Vault", 
        "Enter a strong master password:\n(This will protect all your data)"
    );
    
    if (masterPassword.isEmpty()) {
        return;
    }
    
    QString confirmPassword = getSecurePasswordInput(
        "Confirm Password", 
        "Please confirm your master password:"
    );
    
    if (masterPassword != confirmPassword) {
        showCriticalError("Password Mismatch", "The passwords do not match. Please try again.");
        return;
    }
    
    // Get vault location
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(defaultPath);
    QString vaultPath = QFileDialog::getSaveFileName(
        this, 
        "Save Vault As", 
        defaultPath + "/vault.gpg",
        "Vault Files (*.gpg);;All Files (*)"
    );
    
    if (vaultPath.isEmpty()) {
        return;
    }
    
    if (vault_->createVault(masterPassword.toStdString(), vaultPath.toStdString())) {
        showInfo("Vault Created", 
            "Vault created successfully!\n\n"
            "Your vault is now ready to use. Remember your master password - "
            "it cannot be recovered if lost!");
        showVaultScreen();
    } else {
        showCriticalError("Creation Failed", "Failed to create vault. Please try again.");
    }
}

void MainWindow::onOpenVault() {
    QString vaultPath = QFileDialog::getOpenFileName(
        this, 
        "Open Vault", 
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation),
        "Vault Files (*.gpg);;All Files (*)"
    );
    
    if (vaultPath.isEmpty()) {
        return;
    }
    
    QString masterPassword = getSecurePasswordInput(
        "Open Vault", 
        "Enter your master password:"
    );
    
    if (masterPassword.isEmpty()) {
        return;
    }
    
    if (vault_->openVault(masterPassword.toStdString(), vaultPath.toStdString())) {
        showInfo("Vault Opened", "Vault unlocked successfully!");
        showVaultScreen();
    } else {
        showCriticalError("Authentication Failed", 
            "Incorrect password or corrupted vault file.\n"
            "Please check your password and try again.");
    }
}

void MainWindow::onCreateEntry() {
    if (!vault_->isOpen()) {
        showCriticalError("No Vault", "Please open a vault first.");
        return;
    }
    
    VaultCreationDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        auto entry = dialog.getEntry();
        if (!entry.id.empty()) {
            if (vault_->saveEntry(entry)) {
                showInfo("Entry Saved", 
                    QString("Entry '%1' has been saved to the vault.\n\n"
                            "The credentials were displayed only once and are now encrypted.")
                    .arg(QString::fromStdString(entry.label)));
                updateSecurityStatus();
            } else {
                showCriticalError("Save Failed", "Failed to save entry to vault.");
            }
        }
    }
}

void MainWindow::onViewVault() {
    if (!vault_->isOpen()) {
        showCriticalError("No Vault", "Please open a vault first.");
        return;
    }
    
    VaultViewDialog dialog(vault_.get(), this);
    dialog.exec();
    updateSecurityStatus();
}

void MainWindow::onLockVault() {
    if (vault_->isOpen()) {
        vault_->closeVault();
        showWelcomeScreen();
        showInfo("Vault Locked", "Vault has been locked and all sensitive data cleared from memory.");
    }
}

void MainWindow::onSettings() {
    // Settings functionality coming in future release
    showInfo("Settings", "Settings functionality coming in future release.");
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About Crimson Lock",
        "<b>Crimson Lock v1.0.0</b><br><br>"
        "A secure password manager with hardware-based randomness<br>"
        "and military-grade encryption.<br><br>"
        "<b>Security Features:</b><br>"
        "• Hardware true random number generation<br>"
        "• AES-256 encryption with Argon2id key derivation<br>"
        "• Secure memory management<br>"
        "• Auto-lock functionality<br>"
        "• Offline operation<br><br>"
        "<b>Built with:</b> C++17, Qt 6, libgpgme, libargon2<br><br>"
        "Copyright © 2025 bacpan"
    );
}

void MainWindow::checkAutoLock() {
    if (vault_->isOpen() && vault_->shouldAutoLock()) {
        onLockVault();
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (vault_->isOpen()) {
        vault_->closeVault();
    }
    event->accept();
}

void MainWindow::showCriticalError(const QString& title, const QString& message) {
    QMessageBox::critical(this, title, message);
}

void MainWindow::showInfo(const QString& title, const QString& message) {
    QMessageBox::information(this, title, message);
}

QString MainWindow::getSecurePasswordInput(const QString& title, const QString& prompt) {
    bool ok;
    QString password = QInputDialog::getText(
        this, 
        title, 
        prompt, 
        QLineEdit::Password, 
        "", 
        &ok
    );
    
    return ok ? password : QString();
}

} // namespace ui
} // namespace crimson
