const canvas = document.getElementById('visualizationCanvas');
const ctx = canvas.getContext('2d');

function clearCanvas() {
  ctx.clearRect(0, 0, canvas.width, canvas.height);
}

function drawText(text, x, y, color = 'black', fontSize = '16px', delay = 0) {
  setTimeout(() => {
      ctx.fillStyle = color;
      ctx.font = `${fontSize} Arial`;
      ctx.fillText(text, x, y);
  }, delay);
}

function drawCircle(x, y, radius, color, delay = 0) {
  setTimeout(() => {
      ctx.beginPath();
      ctx.arc(x, y, radius, 0, 2 * Math.PI, false);
      ctx.fillStyle = color;
      ctx.fill();
  }, delay);
}

function drawLine(x1, y1, x2, y2, color, delay = 0) {
  setTimeout(() => {
      ctx.beginPath();
      ctx.moveTo(x1, y1);
      ctx.lineTo(x2, y2);
      ctx.strokeStyle = color;
      ctx.stroke();
  }, delay);
}

function visualizeStep1(p, g) {
  clearCanvas();
  drawText(`Prime (p): ${p}`, 50, 50, 'blue', '20px');
  drawText(`Primitive Root (g): ${g}`, 50, 100, 'blue', '20px', 500);
}

function visualizeStep2(privateKeyAlice, privateKeyBob) {
  clearCanvas();
  drawText(`Alice's Private Key: ${privateKeyAlice}`, 50, 50, 'green', '20px');
  drawText(`Bob's Private Key: ${privateKeyBob}`, 50, 100, 'green', '20px', 500);
}

function visualizeStep3(publicKeyAlice, publicKeyBob) {
  clearCanvas();
  drawText(`Alice's Public Key: ${publicKeyAlice}`, 50, 50, 'orange', '20px');
  drawText(`Bob's Public Key: ${publicKeyBob}`, 50, 100, 'orange', '20px', 500);
}

function visualizeStep4(sharedSecretAlice, sharedSecretBob) {
  clearCanvas();
  drawText(`Shared Secret (Alice): ${sharedSecretAlice}`, 50, 50, 'red', '20px');
  drawText(`Shared Secret (Bob): ${sharedSecretBob}`, 50, 100, 'red', '20px', 500);
}


// Prime number and primitive root generation
function isPrime(num) {
  if (num <= 1) return false;
  if (num <= 3) return true;
  if (num % 2 === 0 || num % 3 === 0) return false;
  for (let i = 5; i * i <= num; i += 6) {
      if (num % i === 0 || num % (i + 2) === 0) return false;
  }
  return true;
}

function generateLargePrime(bits, callback) {
  const min = 2 ** (bits - 1);
  const max = 2 ** bits - 1;
  function findPrime() {
      let prime = Math.floor(Math.random() * (max - min) + min);
      if (isPrime(prime)) {
          callback(prime);
      } else {
          setTimeout(findPrime, 10); // Throttle execution
      }
  }
  findPrime();
}


function findPrimitiveRoot(p) {
  for (let g = 2; g < p; g++) {
      let seen = new Set();
      for (let power = 1; power < p; power++) {
          seen.add(modExp(g, power, p));
      }
      if (seen.size === p - 1) return g;
  }
  throw new Error('Primitive root not found');
}

function generatePrivateKey(p) {
  return Math.floor(Math.random() * (p - 2)) + 1; // private key in range [1, p-1]
}

// Modular exponentiation
function modExp(base, exp, mod) {
  let result = 1;
  base = base % mod;
  while (exp > 0) {
      if (exp % 2 === 1) { // If exp is odd, multiply the base with result
          result = (result * base) % mod;
      }
      exp = Math.floor(exp / 2); // Divide the exp by 2
      base = (base * base) % mod; // Square the base
  }
  return result;
}

// Compute public key
function computePublicKey(g, privateKey, p) {
  return modExp(g, privateKey, p);
}

// Compute shared secret
function computeSharedSecret(publicKey, privateKey, p) {
  return modExp(publicKey, privateKey, p);
}

// Diffie-Hellman Key Exchange implementation
function diffieHellmanKeyExchange() {
  generateLargePrime(16, p => {
      const g = findPrimitiveRoot(p);
      visualizeStep1(p, g);

      setTimeout(() => {
          const privateKeyAlice = generatePrivateKey(p);
          const privateKeyBob = generatePrivateKey(p);
          visualizeStep2(privateKeyAlice, privateKeyBob);

          setTimeout(() => {
              const publicKeyAlice = computePublicKey(g, privateKeyAlice, p);
              const publicKeyBob = computePublicKey(g, privateKeyBob, p);
              visualizeStep3(publicKeyAlice, publicKeyBob);

              setTimeout(() => {
                  const sharedSecretAlice = computeSharedSecret(publicKeyBob, privateKeyAlice, p);
                  const sharedSecretBob = computeSharedSecret(publicKeyAlice, privateKeyBob, p);
                  visualizeStep4(sharedSecretAlice, sharedSecretBob);

                  const output = `
                      <strong>Large Prime (p):</strong> ${p}<br>
                      <strong>Primitive Root (g):</strong> ${g}<br>
                      <strong>Alice's Private Key:</strong> ${privateKeyAlice}<br>
                      <strong>Bob's Private Key:</strong> ${privateKeyBob}<br>
                      <strong>Alice's Public Key:</strong> ${publicKeyAlice}<br>
                      <strong>Bob's Public Key:</strong> ${publicKeyBob}<br>
                      <strong>Shared Secret (Alice):</strong> ${sharedSecretAlice}<br>
                      <strong>Shared Secret (Bob):</strong> ${sharedSecretBob}
                  `;
                  document.getElementById('dhOutput').innerHTML = output;
              }, 2000);
          }, 2000);
      }, 2000);
  });
}




// Function to handle Diffie-Hellman button click
function diffieHellman() {
  document.getElementById('dhOutput').innerHTML = 'Generating keys...';
  diffieHellmanKeyExchange();
}


