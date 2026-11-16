# Validation — 16 November 2026

## Version 0.3.0 project-local Git discovery

- UE **5.4.4** Win64 packaging succeeded. Both Unreal automation tests passed (2 succeeded, 0 failed).
- Added `GitHubBridge.ProjectLocalGitDiscovery` to check all four supported installation layouts, deterministic preference for the `cmd` launcher, paths containing spaces, and fallback when the project has no local executable or only an empty `git` folder. These are location-selection tests; placeholder executable files are never run.
- The existing repository/process test still exercises the actual installed Git executable. A complete project-local PortableGit installation has not been exercised end to end.

## Version 0.2.0 repository detection

- Rebuilt successfully against Unreal Engine **5.4.4** for Win64.
- The expanded `GitHubBridge.RepositoryGuardsAndProcessArguments` test passed in Unreal's automation runner.
- Added real-Git automation coverage for detecting a repository from a nested project folder, importing origin/current branch/author identity, retaining the GitHub account setting, detecting a linked worktree through its `.git` file, reporting detached HEAD without guessing a branch, clearing absent/unsupported origins, excluding credential-bearing URLs, and preserving configuration on invalid repository metadata.
- Detection uses local reads only. It runs in the background at editor startup and from the **Detect project repository** button.

## Original version validation

- Packaged successfully with Unreal Engine **5.4.4**, Win64 Development Editor, using Unreal AutomationTool `BuildPlugin`.
- `GitHubBridge.RepositoryGuardsAndProcessArguments` passed inside Unreal Editor's automation runner (exit code 0).
- The test uses the real Git executable and isolated local fixtures. It covers configuration, unborn branches, quoted commit messages, paths containing spaces, dirty-tree pull/switch refusal, empty commit message refusal, branch mismatch, separate push URL refusal, credential-bearing URL refusal, invalid branches, and subfolder-root refusal.
- A separate editor instance loaded the plugin test project. Visual verification could not be completed because the computer-use screenshot service timed out on both attempts. The settings layout, modal dialog behavior, and all exit choices still require a manual UI check.
- Real GitHub browser sign-in and remote pull/push were not tested. No GitHub repository was modified. The installed Git Credential Manager is older than the browser-login command this plugin requires; use a current Git for Windows/GCM installation or existing SSH authentication.

Suggested account-backed smoke test: use a disposable GitHub repository, configure origin/identity/branch, sign in, modify a project file, inspect status, commit/push, and pull a separate remote commit. Verify that cancelling exit keeps Unreal open, closing without push follows Unreal's save flow, and a rejected push keeps the exit dialog open while retaining the local commit. Restart Unreal after pulling/switching asset revisions.
