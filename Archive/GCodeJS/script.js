document.getElementById('gcode-form').addEventListener('submit', async function (event) {
    event.preventDefault();
    const formData = new FormData(this);
    const response = await fetch('/send-gcode', {
        method: 'POST',
        body: formData
    });
    const data = await response.json();
    const responseDiv = document.getElementById('response-message');
    if (data.success) {
        responseDiv.textContent = data.message;
    } else {
        responseDiv.textContent = 'Error: ' + data.message;
    }
});