#include "ui/VaultCreationDialog.h"
#include "core/PasswordGenerator.h"
#include "core/VaultEntry.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QGroupBox>
#include <QtGui/QClipboard>
#include <QtCore/QTimer>

namespace crimson {
namespace ui {

VaultCreationDialog::VaultCreationDialog(QWidget* parent)
    : QDialog(parent)
    , credentials_generated_(false)
    , entry_saved_(false)
    , password_hide_timer_(new QTimer(this))
    , credentials_widget_(nullptr) {
    
    setupUI();
    
    // Setup password hide timer
    password_hide_timer_->setSingleShot(true);
    connect(password_hide_timer_, &QTimer::timeout, this, &VaultCreationDialog::hidePasswordDisplay);
    
    updateButtonStates();
}

VaultCreationDialog::~VaultCreationDialog() {
    clearSensitiveDisplay();
}

void VaultCreationDialog::setupUI() {
    setWindowTitle("Create New Entry");
    setModal(true);
    resize(600, 500);
    
    main_layout_ = new QVBoxLayout(this);
    main_layout_->setSpacing(20);
    
    // Input section
    auto* input_group = new QGroupBox("Entry Information");
    form_layout_ = new QFormLayout(input_group);
    
    label_input_ = new QLineEdit;
    label_input_->setPlaceholderText("e.g., Gmail Account, Bank Login, etc.");
    form_layout_->addRow("Label:", label_input_);
    
    main_layout_->addWidget(input_group);
    
    // Generate button
    generate_btn_ = new QPushButton("Generate Secure Credentials");
    generate_btn_->setMinimumHeight(50);
    generate_btn_->setStyleSheet(
        "QPushButton { font-size: 16px; background-color: #F18F01; color: white; border: none; border-radius: 8px; padding: 15px; }"
        "QPushButton:hover { background-color: #D17001; }"
        "QPushButton:pressed { background-color: #B15001; }"
        "QPushButton:disabled { background-color: #ccc; color: #999; }"
    );
    connect(generate_btn_, &QPushButton::clicked, this, &VaultCreationDialog::onGenerateCredentials);
    main_layout_->addWidget(generate_btn_);
    
    // Credentials display section
    setupCredentialsDisplay();
    
    // Security warning
    security_warning_ = new QLabel(
        "⚠️ <b>Security Notice:</b> Credentials will be shown only once! "
        "Make sure to save them before closing this dialog."
    );
    security_warning_->setStyleSheet("color: #C73E1D; background-color: #FFF3E0; padding: 10px; border-radius: 5px;");
    security_warning_->setWordWrap(true);
    security_warning_->hide();
    main_layout_->addWidget(security_warning_);
    
    // Action buttons
    button_layout_ = new QHBoxLayout;
    button_layout_->setSpacing(10);
    
    save_btn_ = new QPushButton("Save Entry");
    save_btn_->setMinimumHeight(40);
    save_btn_->setStyleSheet(
        "QPushButton { font-size: 14px; background-color: #2E86AB; color: white; border: none; border-radius: 5px; padding: 10px 20px; }"
        "QPushButton:hover { background-color: #1E5F7A; }"
        "QPushButton:pressed { background-color: #0E3F5A; }"
        "QPushButton:disabled { background-color: #ccc; color: #999; }"
    );
    connect(save_btn_, &QPushButton::clicked, this, &VaultCreationDialog::onSaveEntry);
    
    cancel_btn_ = new QPushButton("Cancel");
    cancel_btn_->setMinimumHeight(40);
    cancel_btn_->setStyleSheet(
        "QPushButton { font-size: 14px; background-color: #666; color: white; border: none; border-radius: 5px; padding: 10px 20px; }"
        "QPushButton:hover { background-color: #555; }"
        "QPushButton:pressed { background-color: #333; }"
    );
    connect(cancel_btn_, &QPushButton::clicked, this, &VaultCreationDialog::onCancel);
    
    button_layout_->addStretch();
    button_layout_->addWidget(save_btn_);
    button_layout_->addWidget(cancel_btn_);
    
    main_layout_->addLayout(button_layout_);
    
    // Connect input validation
    connect(label_input_, &QLineEdit::textChanged, this, &VaultCreationDialog::updateButtonStates);
}

void VaultCreationDialog::setupCredentialsDisplay() {
    credentials_widget_ = new QWidget;
    credentials_layout_ = new QVBoxLayout(credentials_widget_);
    
    auto* creds_group = new QGroupBox("Generated Credentials");
    auto* creds_layout = new QFormLayout(creds_group);
    
    credentials_title_ = new QLabel("<b>Your New Credentials (One-time display)</b>");
    credentials_title_->setStyleSheet("color: #2E86AB; font-size: 16px; padding: 10px;");
    credentials_title_->setAlignment(Qt::AlignCenter);
    
    // Username display
    auto* username_layout = new QHBoxLayout;
    username_display_ = new QLineEdit;
    username_display_->setReadOnly(true);
    username_display_->setStyleSheet("background-color: #f0f0f0; font-family: monospace; font-size: 14px;");
    
    copy_username_btn_ = new QPushButton("Copy");
    copy_username_btn_->setMaximumWidth(80);
    connect(copy_username_btn_, &QPushButton::clicked, this, &VaultCreationDialog::onCopyUsername);
    
    username_layout->addWidget(username_display_);
    username_layout->addWidget(copy_username_btn_);
    
    // Password display
    auto* password_layout = new QHBoxLayout;
    password_display_ = new QLineEdit;
    password_display_->setReadOnly(true);
    password_display_->setEchoMode(QLineEdit::Password);
    password_display_->setStyleSheet("background-color: #f0f0f0; font-family: monospace; font-size: 14px;");
    
    show_password_btn_ = new QPushButton("Show 3s");
    show_password_btn_->setMaximumWidth(80);
    connect(show_password_btn_, &QPushButton::clicked, this, &VaultCreationDialog::onShowPasswordTemporary);
    
    copy_password_btn_ = new QPushButton("Copy");
    copy_password_btn_->setMaximumWidth(80);
    connect(copy_password_btn_, &QPushButton::clicked, this, &VaultCreationDialog::onCopyPassword);
    
    password_layout->addWidget(password_display_);
    password_layout->addWidget(show_password_btn_);
    password_layout->addWidget(copy_password_btn_);
    
    creds_layout->addRow("Username:", username_layout);
    creds_layout->addRow("Password:", password_layout);
    
    credentials_layout_->addWidget(credentials_title_);
    credentials_layout_->addWidget(creds_group);
    
    credentials_widget_->hide();
    main_layout_->addWidget(credentials_widget_);
}

void VaultCreationDialog::onGenerateCredentials() {
    if (!validateInput()) {
        return;
    }
    
    try {
        crimson::core::PasswordGenerator generator;
        
        // Create the entry
        entry_.id = crimson::core::VaultEntry::generateUuid();
        entry_.label = label_input_->text().toStdString();
        entry_.username = generator.generateUsername();
        entry_.password = generator.generatePassword(64, true);
        entry_.created_at = crimson::core::VaultEntry::getCurrentTimestamp();
        entry_.device_fingerprint = crimson::core::VaultEntry::getDeviceFingerprint();
        
        // Display credentials
        username_display_->setText(QString::fromStdString(entry_.username));
        password_display_->setText(QString::fromStdString(entry_.password));
        
        credentials_generated_ = true;
        setCredentialsVisible(true);
        security_warning_->show();
        
        updateButtonStates();
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Generation Failed", 
            QString("Failed to generate credentials: %1").arg(e.what()));
    }
}

void VaultCreationDialog::onSaveEntry() {
    if (!credentials_generated_) {
        QMessageBox::warning(this, "Cannot Save", 
            "Please generate credentials first.");
        return;
    }
    
    entry_saved_ = true;
    accept();
}

void VaultCreationDialog::onCancel() {
    if (credentials_generated_ && !entry_saved_) {
        int ret = QMessageBox::warning(this, "Unsaved Entry",
            "You have generated credentials but haven't saved the entry.\n"
            "The credentials will be lost forever. Are you sure you want to cancel?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        
        if (ret != QMessageBox::Yes) {
            return;
        }
    }
    
    clearSensitiveDisplay();
    reject();
}

void VaultCreationDialog::onCopyUsername() {
    copyToClipboard(username_display_->text(), "username");
}

void VaultCreationDialog::onCopyPassword() {
    copyToClipboard(password_display_->text(), "password");
}

void VaultCreationDialog::onShowPasswordTemporary() {
    password_display_->setEchoMode(QLineEdit::Normal);
    show_password_btn_->setEnabled(false);
    show_password_btn_->setText("Hiding...");
    
    // Hide password after 3 seconds
    password_hide_timer_->start(3000);
}

void VaultCreationDialog::hidePasswordDisplay() {
    password_display_->setEchoMode(QLineEdit::Password);
    show_password_btn_->setEnabled(true);
    show_password_btn_->setText("Show 3s");
}

void VaultCreationDialog::setCredentialsVisible(bool visible) {
    credentials_widget_->setVisible(visible);
    if (visible) {
        adjustSize();
    }
}

void VaultCreationDialog::updateButtonStates() {
    bool hasLabel = !label_input_->text().trimmed().isEmpty();
    bool canGenerate = hasLabel && !credentials_generated_;
    bool canSave = credentials_generated_;
    
    generate_btn_->setEnabled(canGenerate);
    save_btn_->setEnabled(canSave);
    
    if (credentials_generated_) {
        label_input_->setEnabled(false);
        generate_btn_->setText("Credentials Generated");
    }
}

bool VaultCreationDialog::validateInput() {
    QString label = label_input_->text().trimmed();
    
    if (label.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a label for the entry.");
        label_input_->setFocus();
        return false;
    }
    
    if (label.length() > 100) {
        QMessageBox::warning(this, "Invalid Input", "Label cannot be longer than 100 characters.");
        label_input_->setFocus();
        return false;
    }
    
    return true;
}

void VaultCreationDialog::clearSensitiveDisplay() {
    if (username_display_) {
        username_display_->clear();
    }
    if (password_display_) {
        password_display_->clear();
    }
    
    // Clear from clipboard if it might contain our data
    QApplication::clipboard()->clear();
}

void VaultCreationDialog::copyToClipboard(const QString& text, const QString& description) {
    QApplication::clipboard()->setText(text);
    
    // Show temporary message
    QMessageBox::information(this, "Copied", 
        QString("The %1 has been copied to clipboard.\n"
                "Clipboard will be cleared automatically in 30 seconds for security.").arg(description));
    
    // Clear clipboard after 30 seconds
    QTimer::singleShot(30000, []() {
        QApplication::clipboard()->clear();
    });
}

} // namespace ui
} // namespace crimson
