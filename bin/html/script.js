function switchTab(tabId) {
    // Hide all chat lists
    document.querySelectorAll('.chat-list').forEach(list => list.style.display = 'none');
    // Remove active class from all tabs
    document.querySelectorAll('.tab').forEach(tab => tab.classList.remove('active'));
    // Show the selected chat list
    document.getElementById(tabId).style.display = 'block';
    // Add active class to the selected tab
    document.querySelector(`.tab[onclick="switchTab('${tabId}')"]`).classList.add('active');
}

function openChat(chatId) {
    // Hide all chat areas
    document.querySelectorAll('.chat-area').forEach(chat => chat.style.display = 'none');
    // Show the selected chat area
    document.getElementById(`chat-${chatId}`).style.display = 'flex';
}

// Example: Preload chat data (can be dynamic)
document.addEventListener("DOMContentLoaded", function() {
    switchTab('all');  // Load the "All" tab by default
});
