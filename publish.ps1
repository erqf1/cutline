# Publishes Cutline to GitHub: creates nothing by itself, you only click "Create repository" once.
# Uses your normal git login (Git Credential Manager opens the browser if needed).
$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot
$user = (Read-Host "Your GitHub username").Trim()
if (-not $user) { throw "No username given." }
$repo = "cutline"

Write-Host ""
Write-Host "1) A browser tab opens with the new-repository form (name '$repo', public)."
Write-Host "   Click 'Create repository' (leave README / .gitignore / license UNCHECKED)."
Start-Process "https://github.com/new?name=$repo&visibility=public&description=Simple%2C%20fast%20video%20editor%20and%20player"
Read-Host "Press Enter when the empty repository exists"

git remote remove origin 2>$null
git remote add origin "https://github.com/$user/$repo.git"
git branch -M main
git push -u origin main
git tag -f v1.0.0
git push -f origin v1.0.0

Write-Host ""
Write-Host "2) Pushed. GitHub Actions now builds all downloads (about 15-25 minutes):"
Write-Host "   https://github.com/$user/$repo/actions"
Start-Process "https://github.com/$user/$repo/actions"
Write-Host ""
Write-Host "3) Website: in the next tab choose  Source = 'Deploy from a branch',  Branch = main,  Folder = /docs,  then Save."
Write-Host "   Your site will be:  https://$user.github.io/$repo/"
Start-Process "https://github.com/$user/$repo/settings/pages"
Read-Host "Done - press Enter to close"
