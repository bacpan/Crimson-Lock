#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QGroupBox>
#include <QtCore/QTimer>
#include <memory>
#include "../core/SecureVault.h"

namespace crimson {
namespace ui {

/**
 * @brief Dialog for viewing and managing vault entries
 * 
 * Provides:
 * - List of all vault entries (labels only)
 * - Entry details view (username visible, password hidden)
 * - Secure password reveal and copy functionality
 * - Entry deletion
 */
class VaultViewDialog : public QDialog {
    Q_OBJECT

public:
    explicit VaultViewDialog(crimson::core::SecureVault* vault, QWidget* parent = nullptr);
    ~VaultViewDialog();

private slots:
    void onEntrySelectionChanged();
    void onRefreshEntries();
    void onDeleteEntry();
    void onCopyUsername();
    void onCopyPassword();
    void onShowPasswordTemporary();
    void onClose();
    void hidePasswordDisplay();
    void clearClipboard();

private:
    // Core components
    crimson::core::SecureVault* vault_;
    std::string current_entry_id_;
    
    // UI components
    QVBoxLayout* main_layout_;
    QHBoxLayout* button_layout_;
    QSplitter* splitter_;
    
    // Entry list
    QGroupBox* list_group_;
    QVBoxLayout* list_layout_;
    QListWidget* entry_list_;
    QPushButton* refresh_btn_;
    QPushButton* delete_btn_;
    
    // Entry details
    QGroupBox* details_group_;
    QVBoxLayout* details_layout_;
    QLabel* entry_id_label_;
    QLabel* entry_label_display_;
    QLabel* username_label_;
    QLineEdit* username_display_;
    QLabel* password_label_;
    QLineEdit* password_display_;
    QLabel* created_at_display_;
    
    // Action buttons for details
    QHBoxLayout* details_actions_layout_;
    QPushButton* copy_username_btn_;
    QPushButton* copy_password_btn_;
    QPushButton* show_password_btn_;
    
    // Security features
    QTimer* password_hide_timer_;
    QTimer* clipboard_clear_timer_;
    QLabel* security_status_;
    
    // Main dialog buttons
    QPushButton* close_btn_;
    
    /**
     * @brief Initialize UI components
     */
    void setupUI();
    
    /**
     * @brief Setup entry list widget
     */
    void setupEntryList();
    
    /**
     * @brief Setup entry details widget
     */
    void setupEntryDetails();
    
    /**
     * @brief Load all entries into the list
     */
    void loadEntries();
    
    /**
     * @brief Display details for selected entry
     */
    void displayEntryDetails(const std::string& entryId);
    
    /**
     * @brief Clear entry details display
     */
    void clearEntryDetails();
    
    /**
     * @brief Update button states based on selection
     */
    void updateButtonStates();
    
    /**
     * @brief Copy text to clipboard with auto-clear
     */
    void copyToClipboard(const QString& text, const QString& description);
    
    /**
     * @brief Clear all sensitive display data
     */
    void clearSensitiveDisplay();
    
    /**
     * @brief Show confirmation dialog
     */
    bool showConfirmation(const QString& title, const QString& message);
};

} // namespace ui
} // namespace crimson
