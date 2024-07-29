function showLoginPage() {
    document.getElementById('register-page').style.display = 'none';
    document.getElementById('login-page').style.display = 'block';
}

function showRegisterPage() {
    document.getElementById('login-page').style.display = 'none';
    document.getElementById('register-page').style.display = 'block';
}

function login() {
    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;

    // Add logic to verify username and password
    // For now, just show the chat page
    if (username && password) {
        document.getElementById('login-page').style.display = 'none';
        document.getElementById('chat-page').style.display = 'flex';
    } else {
        alert('Please enter a username and password');
    }
}

function registerUser() {
    const username = document.getElementById('reg-username').value;
    const nickname = document.getElementById('reg-nickname').value;
    const password = document.getElementById('reg-password').value;
    const email = document.getElementById('reg-email').value;

    if (username && nickname && password && email) {
        const ws = new WebSocket('ws://127.0.0.1:8020/tao/chat');

        ws.onopen = function() {
            console.log('WebSocket connection established');
            ws.send(JSON.stringify({ type: 'register', username, nickname, password, email }));
        };

        ws.onmessage = function(event) {
            const response = JSON.parse(event.data);
            console.log('Received from server:', response);
            document.getElementById('register-msg').innerText = response.msg;

            if (response.msg === 'ok') {
                document.getElementById('register-page').style.display = 'none';
                document.getElementById('chat-page').style.display = 'flex';
            }
        };

        ws.onerror = function(error) {
            console.error('WebSocket error:', error);
            document.getElementById('register-msg').innerText = 'WebSocket connection failed';
        };
    } else {
        alert('Please fill in all fields');
    }
}

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
