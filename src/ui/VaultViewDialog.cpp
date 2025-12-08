#include "ui/VaultViewDialog.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QHeaderView>
#include <QtGui/QClipboard>

namespace crimson {
namespace ui {

VaultViewDialog::VaultViewDialog(crimson::core::SecureVault* vault, QWidget* parent)
    : QDialog(parent)
    , vault_(vault)
    , password_hide_timer_(new QTimer(this))
    , clipboard_clear_timer_(new QTimer(this)) {
    
    setupUI();
    loadEntries();
    
    // Setup timers
    password_hide_timer_->setSingleShot(true);
    connect(password_hide_timer_, &QTimer::timeout, this, &VaultViewDialog::hidePasswordDisplay);
    
    clipboard_clear_timer_->setSingleShot(true);
    connect(clipboard_clear_timer_, &QTimer::timeout, this, &VaultViewDialog::clearClipboard);
    
    updateButtonStates();
}

VaultViewDialog::~VaultViewDialog() {
    clearSensitiveDisplay();
}

void VaultViewDialog::setupUI() {
    setWindowTitle("Vault Entries");
    setModal(true);
    resize(900, 600);
    
    main_layout_ = new QVBoxLayout(this);
    main_layout_->setSpacing(15);
    
    // Create splitter for list and details
    splitter_ = new QSplitter(Qt::Horizontal);
    
    setupEntryList();
    setupEntryDetails();
    
    splitter_->addWidget(list_group_);
    splitter_->addWidget(details_group_);
    splitter_->setStretchFactor(0, 1);
    splitter_->setStretchFactor(1, 2);
    
    main_layout_->addWidget(splitter_);
    
    // Security status
    security_status_ = new QLabel("Vault entries are encrypted and secure");
    security_status_->setStyleSheet("color: #666; font-style: italic; padding: 5px;");
    security_status_->setAlignment(Qt::AlignCenter);
    main_layout_->addWidget(security_status_);
    
    // Dialog buttons
    button_layout_ = new QHBoxLayout;
    
    close_btn_ = new QPushButton("Close");
    close_btn_->setMinimumHeight(35);
    close_btn_->setStyleSheet(
        "QPushButton { font-size: 14px; background-color: #666; color: white; border: none; border-radius: 5px; padding: 8px 16px; }"
        "QPushButton:hover { background-color: #555; }"
    );
    connect(close_btn_, &QPushButton::clicked, this, &VaultViewDialog::onClose);
    
    button_layout_->addStretch();
    button_layout_->addWidget(close_btn_);
    
    main_layout_->addLayout(button_layout_);
}

void VaultViewDialog::setupEntryList() {
    list_group_ = new QGroupBox("Vault Entries");
    list_layout_ = new QVBoxLayout(list_group_);
    
    // Entry list
    entry_list_ = new QListWidget;
    entry_list_->setStyleSheet(
        "QListWidget { font-size: 14px; }"
        "QListWidget::item { padding: 10px; border-bottom: 1px solid #eee; }"
        "QListWidget::item:selected { background-color: #2E86AB; color: white; }"
        "QListWidget::item:hover { background-color: #f0f8ff; }"
    );
    connect(entry_list_, &QListWidget::currentRowChanged, this, &VaultViewDialog::onEntrySelectionChanged);
    
    list_layout_->addWidget(entry_list_);
    
    // List action buttons
    auto* list_buttons = new QHBoxLayout;
    
    refresh_btn_ = new QPushButton("Refresh");
    refresh_btn_->setStyleSheet(
        "QPushButton { background-color: #F18F01; color: white; border: none; border-radius: 3px; padding: 5px 10px; }"
        "QPushButton:hover { background-color: #D17001; }"
    );
    connect(refresh_btn_, &QPushButton::clicked, this, &VaultViewDialog::onRefreshEntries);
    
    delete_btn_ = new QPushButton("Delete");
    delete_btn_->setStyleSheet(
        "QPushButton { background-color: #C73E1D; color: white; border: none; border-radius: 3px; padding: 5px 10px; }"
        "QPushButton:hover { background-color: #A73E1D; }"
        "QPushButton:disabled { background-color: #ccc; color: #999; }"
    );
    connect(delete_btn_, &QPushButton::clicked, this, &VaultViewDialog::onDeleteEntry);
    
    list_buttons->addWidget(refresh_btn_);
    list_buttons->addStretch();
    list_buttons->addWidget(delete_btn_);
    
    list_layout_->addLayout(list_buttons);
}

void VaultViewDialog::setupEntryDetails() {
    details_group_ = new QGroupBox("Entry Details");
    details_layout_ = new QVBoxLayout(details_group_);
    
    // Entry info
    entry_label_display_ = new QLabel;
    entry_label_display_->setStyleSheet("font-size: 18px; font-weight: bold; color: #2E86AB; padding: 10px;");
    entry_label_display_->setAlignment(Qt::AlignCenter);
    entry_label_display_->setWordWrap(true);
    
    details_layout_->addWidget(entry_label_display_);
    
    // Details form
    auto* form_widget = new QWidget;
    auto* form_layout = new QFormLayout(form_widget);
    form_layout->setSpacing(15);
    
    // Username
    auto* username_layout = new QHBoxLayout;
    username_display_ = new QLineEdit;
    username_display_->setReadOnly(true);
    username_display_->setStyleSheet("background-color: #f8f8f8; font-family: monospace; font-size: 12px; padding: 8px;");
    
    copy_username_btn_ = new QPushButton("Copy");
    copy_username_btn_->setMaximumWidth(60);
    copy_username_btn_->setStyleSheet(
        "QPushButton { background-color: #2E86AB; color: white; border: none; border-radius: 3px; padding: 5px; }"
        "QPushButton:hover { background-color: #1E5F7A; }"
        "QPushButton:disabled { background-color: #ccc; color: #999; }"
    );
    connect(copy_username_btn_, &QPushButton::clicked, this, &VaultViewDialog::onCopyUsername);
    
    username_layout->addWidget(username_display_);
    username_layout->addWidget(copy_username_btn_);
    
    // Password
    auto* password_layout = new QHBoxLayout;
    password_display_ = new QLineEdit;
    password_display_->setReadOnly(true);
    password_display_->setEchoMode(QLineEdit::Password);
    password_display_->setStyleSheet("background-color: #f8f8f8; font-family: monospace; font-size: 12px; padding: 8px;");
    password_display_->setText("••••••••••••••••");
    
    show_password_btn_ = new QPushButton("Show 3s");
    show_password_btn_->setMaximumWidth(70);
    show_password_btn_->setStyleSheet(
        "QPushButton { background-color: #F18F01; color: white; border: none; border-radius: 3px; padding: 5px; }"
        "QPushButton:hover { background-color: #D17001; }"
        "QPushButton:disabled { background-color: #ccc; color: #999; }"
    );
    connect(show_password_btn_, &QPushButton::clicked, this, &VaultViewDialog::onShowPasswordTemporary);
    
    copy_password_btn_ = new QPushButton("Copy");
    copy_password_btn_->setMaximumWidth(60);
    copy_password_btn_->setStyleSheet(
        "QPushButton { background-color: #A23B72; color: white; border: none; border-radius: 3px; padding: 5px; }"
        "QPushButton:hover { background-color: #7A2B52; }"
        "QPushButton:disabled { background-color: #ccc; color: #999; }"
    );
    connect(copy_password_btn_, &QPushButton::clicked, this, &VaultViewDialog::onCopyPassword);
    
    password_layout->addWidget(password_display_);
    password_layout->addWidget(show_password_btn_);
    password_layout->addWidget(copy_password_btn_);
    
    // Created date
    created_at_display_ = new QLabel;
    created_at_display_->setStyleSheet("color: #666; font-size: 12px;");
    
    form_layout->addRow("Username:", username_layout);
    form_layout->addRow("Password:", password_layout);
    form_layout->addRow("Created:", created_at_display_);
    
    details_layout_->addWidget(form_widget);
    details_layout_->addStretch();
    
    clearEntryDetails();
}

void VaultViewDialog::loadEntries() {
    entry_list_->clear();
    
    try {
        auto labels = vault_->getEntryLabels();
        
        for (const auto& [id, label] : labels) {
            auto* item = new QListWidgetItem(QString::fromStdString(label));
            item->setData(Qt::UserRole, QString::fromStdString(id));
            item->setToolTip(QString("Entry ID: %1").arg(QString::fromStdString(id)));
            entry_list_->addItem(item);
        }
        
        security_status_->setText(
            QString("%1 entries loaded • All passwords encrypted and secure")
            .arg(labels.size())
        );
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Load Error", 
            QString("Failed to load entries: %1").arg(e.what()));
    }
}

void VaultViewDialog::onEntrySelectionChanged() {
    auto* current = entry_list_->currentItem();
    if (current) {
        QString entryId = current->data(Qt::UserRole).toString();
        displayEntryDetails(entryId.toStdString());
    } else {
        clearEntryDetails();
    }
    
    updateButtonStates();
}

void VaultViewDialog::displayEntryDetails(const std::string& entryId) {
    try {
        current_entry_id_ = entryId;
        auto entry = vault_->getEntry(entryId);
        
        entry_label_display_->setText(QString(" %1").arg(QString::fromStdString(entry.label)));
        username_display_->setText(QString::fromStdString(entry.username));
        created_at_display_->setText(QString::fromStdString(entry.created_at));
        
        // Reset password display
        password_display_->setText("••••••••••••••••");
        password_display_->setEchoMode(QLineEdit::Password);
        show_password_btn_->setText("Show 3s");
        show_password_btn_->setEnabled(true);
        
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Display Error", 
            QString("Failed to display entry details: %1").arg(e.what()));
        clearEntryDetails();
    }
}

void VaultViewDialog::clearEntryDetails() {
    current_entry_id_.clear();
    entry_label_display_->setText("Select an entry to view details");
    username_display_->clear();
    password_display_->clear();
    created_at_display_->clear();
}

void VaultViewDialog::onRefreshEntries() {
    loadEntries();
    clearEntryDetails();
}

void VaultViewDialog::onDeleteEntry() {
    if (current_entry_id_.empty()) {
        return;
    }
    
    auto* current = entry_list_->currentItem();
    if (!current) {
        return;
    }
    
    QString label = current->text();
    
    if (showConfirmation("Delete Entry", 
        QString("Are you sure you want to delete the entry '%1'?\n\nThis action cannot be undone.").arg(label))) {
        
        try {
            if (vault_->deleteEntry(current_entry_id_)) {
                QMessageBox::information(this, "Entry Deleted", "Entry has been permanently deleted.");
                loadEntries();
                clearEntryDetails();
            } else {
                QMessageBox::critical(this, "Delete Failed", "Failed to delete entry.");
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Delete Error", 
                QString("Failed to delete entry: %1").arg(e.what()));
        }
    }
}

void VaultViewDialog::onCopyUsername() {
    if (!current_entry_id_.empty()) {
        copyToClipboard(username_display_->text(), "username");
    }
}

void VaultViewDialog::onCopyPassword() {
    if (current_entry_id_.empty()) {
        return;
    }
    
    try {
        QString password = QString::fromStdString(vault_->getPassword(current_entry_id_));
        copyToClipboard(password, "password");
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Access Error", 
            QString("Failed to access password: %1").arg(e.what()));
    }
}

void VaultViewDialog::onShowPasswordTemporary() {
    if (current_entry_id_.empty()) {
        return;
    }
    
    try {
        QString password = QString::fromStdString(vault_->getPassword(current_entry_id_));
        
        password_display_->setText(password);
        password_display_->setEchoMode(QLineEdit::Normal);
        show_password_btn_->setEnabled(false);
        show_password_btn_->setText("Hiding...");
        
        // Hide password after 3 seconds
        password_hide_timer_->start(3000);
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Access Error", 
            QString("Failed to access password: %1").arg(e.what()));
    }
}

void VaultViewDialog::hidePasswordDisplay() {
    password_display_->setText("••••••••••••••••");
    password_display_->setEchoMode(QLineEdit::Password);
    show_password_btn_->setEnabled(true);
    show_password_btn_->setText("Show 3s");
}

void VaultViewDialog::onClose() {
    clearSensitiveDisplay();
    accept();
}

void VaultViewDialog::updateButtonStates() {
    bool hasSelection = !current_entry_id_.empty();
    
    delete_btn_->setEnabled(hasSelection);
    copy_username_btn_->setEnabled(hasSelection);
    copy_password_btn_->setEnabled(hasSelection);
    show_password_btn_->setEnabled(hasSelection);
}

void VaultViewDialog::copyToClipboard(const QString& text, const QString& description) {
    QApplication::clipboard()->setText(text);
    
    security_status_->setText(
        QString("📋 %1 copied to clipboard (auto-clear in 30s)").arg(description.toUpper())
    );
    
    // Clear clipboard after 30 seconds
    clipboard_clear_timer_->start(30000);
}

void VaultViewDialog::clearClipboard() {
    QApplication::clipboard()->clear();
    security_status_->setText("Clipboard cleared for security");
}

void VaultViewDialog::clearSensitiveDisplay() {
    if (password_display_) {
        password_display_->clear();
    }
}

bool VaultViewDialog::showConfirmation(const QString& title, const QString& message) {
    return QMessageBox::question(this, title, message, 
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
}

} // namespace ui
} // namespace crimson
