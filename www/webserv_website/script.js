// Ensure this matches your config location path (/upload/ or /uploads/)
const UPLOAD_DIR = "/upload/";
const LIMITED_UPLOAD_DIR = "/limited-upload";

// --- 5. Raw Upload Handler (No multipart/form-data) -------------------
async function handleRawUpload(event) {
  event.preventDefault();

  const fileInput = document.getElementById("raw-file-input");
  const statusDiv = document.getElementById("upload-status");

  if (!fileInput.files.length) {
    if (statusDiv) statusDiv.textContent = "Please select a file first.";
    return false;
  }

  const file = fileInput.files[0];
  if (statusDiv) statusDiv.textContent = "Uploading...";

  try {
    // Sends pure raw bytes/text directly to the file's target URL
    const uploadUrl = `${UPLOAD_DIR}${encodeURIComponent(file.name)}`;
    const res = await fetch(uploadUrl, {
      method: "POST",
      headers: {
        // Sets MIME type or defaults to raw binary stream
        "Content-Type": file.type || "application/octet-stream"
      },
      body: file // Sends the raw File/Blob payload directly
    });

    console.log(`POST ${uploadUrl} -> ${res.status}`);

    if (res.ok || res.status === 201) {
      if (statusDiv) statusDiv.textContent = `Upload successful! (${res.status})`;
      loadFiles(); // Refresh directory list automatically
    } else {
      if (statusDiv) statusDiv.textContent = `Upload failed with status: ${res.status}`;
    }
  } catch (err) {
    console.error("Upload error:", err);
    if (statusDiv) statusDiv.textContent = `Upload error: ${err.message}`;
  }

  return false;
}

// --- 6. List + delete files -------------------------------------------
// Parses the server's autoindex HTML to get filenames. If your autoindex
// markup differs, tweak the selector below (it just grabs <a href="...">).
async function loadFiles() {
  const listEl = document.getElementById("file-list");
  listEl.innerHTML = "<li>loading…</li>";

  try {
    const res = await fetch(UPLOAD_DIR);
    if (!res.ok) throw new Error(`GET ${UPLOAD_DIR} -> ${res.status}`);
    const html = await res.text();

    const doc = new DOMParser().parseFromString(html, "text/html");
    const links = [...doc.querySelectorAll("a")]
      .map(a => a.getAttribute("href"))
      .filter(href => href && !href.endsWith("/") && href !== "../");

    listEl.innerHTML = links.length === 0 ? "<li>(no files)</li>" : "";
    links.forEach(href => {
      const li = document.createElement("li");
      const displayName = href.split("/").filter(Boolean).pop(); // just for display
      li.innerHTML = `<span>${displayName}</span>`;
      const btn = document.createElement("button");
      btn.className = "delete-btn";
      btn.textContent = "Delete";
      btn.onclick = () => deleteFile(href); // use href AS-IS, don't rebuild it
      li.appendChild(btn);
      listEl.appendChild(li);
    });
  } catch (err) {
    listEl.innerHTML = `<li>error loading list: ${err.message}</li>`;
  }
}

async function deleteFile(href) {
  // href is already a valid path (absolute or relative) from the server — use it directly
  const path = href.startsWith("/") ? href : UPLOAD_DIR + href;
  try {
    const res = await fetch(path, { method: "DELETE" });
    console.log(`DELETE ${path} -> ${res.status}`);
    if (res.ok || res.status === 204) loadFiles();
    else alert(`Delete failed: ${res.status} ${res.statusText}`);
  } catch (err) {
    alert(`Delete request failed: ${err.message}`);
  }
}
// --- 4. Wrong method on a GET-only location -> expect 405 --------------
async function testWrongMethod(event) {
  event.preventDefault();
  const res = await fetch("/get-only/", { method: "POST" });
  alert(`POST /get-only/ -> ${res.status} ${res.statusText}`);
  return false;
}

document.addEventListener("DOMContentLoaded", loadFiles);
