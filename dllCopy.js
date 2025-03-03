const { cp } = require('fs').promises;

async function copyDlls() {
  try {
    // Copy a directory recursively
    await cp('./dll', './prebuilds\\win32-x64', { recursive: true });
    console.log('Dlls copied to prebuilds location');
  } catch (err) {
    console.error('Error:', err.message);
  }
}


copyDlls()