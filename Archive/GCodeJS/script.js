  const GCODE_PEN_UP = "G01 Z1 F500";
  const GCODE_PEN_DOWN = "G01 Z-1 F500";

  async function send_gcode(gcode_content) {
      if (gcode_content !== "none") {
          try {
              const response = await fetch('/send-gcode', {
                  method: 'POST',
                  headers: {
                      'Content-Type': 'application/json'
                  },
                  body: JSON.stringify({
                      gcode: gcode_content
                  })
              });
              const data = await response.json();
              const current_text = document.querySelector('#response-message').innerText;

              document.querySelector('#response-message').innerText = current_text + "\n" + data
                  .message;
          } catch (error) {
              console.error('Error sending G-code:', error);
              document.querySelector('#response-message').innerText = 'Error sending G-code';
          }
      }
  }

  /***************************
   ** EVENT HANDLERS
   ****************************/

  document.querySelector('#gcode-form').addEventListener('submit', async (event) => {
      event.preventDefault();
      const formData = new FormData(event.target);
      const lines = formData.get('gcode').split("\n");

      //console.log(lines)
      send_gcode(lines[0])

      // TBD ...
      // Function to send the next line recursively
      // async function sendNextLine(index) {
      //     // Check if there are more lines to send
      //     if (index < lines.length) {
      //         var line = lines[index];
      //         try {
      //             const response = await fetch('/send-gcode', {
      //                 method: 'POST',
      //                 headers: {
      //                     'Content-Type': 'application/json'
      //                 },
      //                 body: JSON.stringify({
      //                     gcode: line
      //                 })
      //             });
      //             const data = await response.json();
      //             const current_text = document.querySelector('#response-message').innerText;

      //             document.querySelector('#response-message').innerText = current_text + "\n" +
      //                 data.message;
      //                 sendNextLine(index + 1);
      //         } catch (error) {
      //             console.error('Error sending G-code:', error);
      //             document.querySelector('#response-message').innerText = 'Error sending G-code';
      //         }

      //         // Send the line to the serial port
      //         // serialPort.write(line + '\n', function (err) {
      //         //     if (err) {
      //         //         console.error('Error writing to serial port:', err.message);
      //         //     } else {
      //         //         console.log('Sent G-code line:', line);
      //         //         // todo
      //         //         // // Wait for serial port response before sending next line
      //         //         // serialPort.once('data', function (data) {
      //         //         //     // Check if the response contains "OK"
      //         //         //     if (data.toString().includes('OK')) {
      //         //         //         // Send the next line
      //         //         //         sendNextLine(index + 1);
      //         //         //     }
      //         //         // });
      //         //     }
      //         // });
      //     } else {
      //         console.log('All lines sent successfully');
      //     }
      // }

      // // Start sending the lines from the first line
      // sendNextLine(0);
      //send_gcode(gcode)
  });

  document.querySelector(".servo_down").addEventListener('click', async (event) => {
      event.preventDefault();
      send_gcode(GCODE_PEN_DOWN);
  });
  document.querySelector(".servo_up").addEventListener('click', async (event) => {
      event.preventDefault();
      send_gcode(GCODE_PEN_UP);
  });
  document.querySelector(".drop_down").addEventListener('change', async (event) => {
      event.preventDefault();
      send_gcode(document.querySelector(".drop_down").value);
  });

  /***************************
   ** WEB SOCKETS
   ****************************/
  const ws = new WebSocket('ws://localhost:8000');

  ws.onopen = () => {
      console.log('WebSocket connection established');
  };

  ws.onmessage = (event) => {
      console.log('Message received from server:', event.data);
      outputDiv.textContent = event.data;
  };

  ws.onerror = (error) => {
      console.error('WebSocket error:', error);
  };

  ws.onclose = () => {
      console.log('WebSocket connection closed');
  };