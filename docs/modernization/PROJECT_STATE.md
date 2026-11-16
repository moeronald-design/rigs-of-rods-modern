# Rigs of Rods Modernization — Project State & Handoff

Last updated: 2026-11-16 (Session 1: initial research)
Status: RESEARCH PHASE. No code written. No decisions locked.

---

## 0. Working rules (owner request)

- Everything we learn, decide, or do goes into THIS file, so later chats read the file instead of the chat history (saves tokens).
- Update this file at every milestone; add a line to the Session log below.
- Keep findings as facts + source/command used, so they can be re-verified.

### Session log
- S1: Researched RoR engine/license/physics; options A-D written (sections 2-5).
- S1: Owner chose direction: upstream-style modernization, PBR + optimize + multiplayer (5b). Provisional: Option A.
- S1b: Cloned repo (commit ebf31a2, 2026-10-19). Found OGRE 1.11.6, not 13/14. Sized OGRE surface and shaders (5c).
- S1c: Build not possible in session (no OGRE libs, no network). Scanned source for OGRE-version-sensitive APIs (5d). Confirmed Option A = version bump, Option B = rewrite.
- S1d: Owner on mobile; chose GitHub Actions CI via a fork; target = BeamNG realism (5e). GitHub not yet linked to Claude (blocker).
- S1e: Fork created and linked; push works. Branch `modern/project-setup` pushed with this file at docs/modernization/PROJECT_STATE.md (the repo copy is now the canonical one). Read upstream AI_POLICY (5f). Fork already has CI (build-game.yml).
- S1f: Owner says maintainers are OK with the effort (5g). Fork has 0 Actions runs so far.
- Next: owner enables Actions on fork and confirms a baseline Linux build passes; inventory add-ons' OGRE 14 compatibility; start `modern/ogre14-port` branch; PBR spike plan.

---

## 1. Goal

Give Rigs of Rods (RoR) a 2026-quality look, closer to BeamNG.drive, as a community project.
Constraint from owner: keep the original physics and gameplay intact. Change only the rendering/engine layer if possible.

---

## 2. Verified facts (Session 1)

| Topic | Finding | Confidence |
|---|---|---|
| License | GPL-3.0, free and open source | High |
| Languages | C++ (core), AngelScript (scripting; replaced Lua at v0.38) | High |
| Rendering | OGRE 1.11.6 (2019-era), via a Conan fork "ogre3d/1.11.6.1@anotherfoxguy/stable"; CMake requires OGRE 1.11 | VERIFIED from repo (Session 1b) |
| Latest stable | RoR 2026.01 (released 3 Jan 2026, first stable since 2022.12; RC builds 2025.03, 2025.08 in between) | High |
| Dev builds | Automated builds on itch.io (rigs-of-rods/rigs-of-rods-dev) | High |
| Physics | Custom node/beam mass-spring-damper soft-body engine, written in-house. NOT Bullet or PhysX | High |
| Physics threading | Docs (2023.09) say physics runs on a dedicated thread. A 2018 dev post called the codebase "single-and-half-threaded" with fat, cache-unfriendly softbody structs and several overlapping collision systems | Medium (old post) |
| Audio | OpenAL Soft | High |
| GUI | Dear ImGui | Medium |
| Multiplayer | RoRnet 2.45 (2026.01 not compatible with 2.44 servers) | High |
| Repo | github.com/RigsOfRods/rigs-of-rods (C++), plus ror-server, docs, multiplayer API | High |
| History | BeamNG.drive was started in 2011 by some RoR developers, as a separate proprietary game | High |
| Latest OGRE | OGRE 14.5.2 (31 Jan 2026). OGRE-Next 3.0 (Oct 2024), 4.0 in development | High |

Not verified (forum.rigsofrods.org blocked automated fetches, "Anubis" bot wall):
- Exact OGRE version RoR 2026.01 links against
- Full 2026.01 changelog (graphics-related items)
- Full text of the forum thread "checklist for porting to different 3d engine than ogre"

---

## 3. Key insight: "What engine does RoR run on?"

RoR is not built on a game engine. OGRE is a rendering library only (scene graph, materials, shaders, meshes, resource loading). RoR itself implements:
- the physics (node/beam solver, collisions, wheels, engines, aerodynamics)
- input, audio integration, terrain loading, vehicle file parsing, scripting host, networking, GUI

So "changing the engine" does not touch the physics. The physics is already independent of the renderer. This is good news for the "keep physics original" constraint.

Lead dev (ohlidalp) position, from the forum port-checklist thread: porting to another 3D engine "would gain us nothing" because the visual bottleneck is assets (materials, shaders, textures), not the technology. Worth taking seriously: much of the BeamNG look gap is art and lighting, not API.

---

## 4. Options considered

### A. Stay on OGRE, upgrade version + modernize assets (lowest risk)
- Move to OGRE 14.x (Vulkan render system exists; mesh shader support added in 14.3).
- Replace legacy materials with PBR (metal/roughness) via RTSS or custom shaders.
- Add modern lighting: better shadows, SSR/SSAO, HDR + tonemapping, volumetric/atmosphere, proper skybox/IBL.
- Re-author hero vehicle and terrain materials.
- Pros: no rewrite, community mods mostly keep working, real contributions can merge upstream.
- Cons: OGRE 1.x-line renderer is forward-renderer-centric; ceiling is lower than OGRE-Next.

### B. Port renderer to OGRE-Next (medium-high risk)
- Hlms PBS, Vulkan/D3D11/Metal/GL, better lighting and global illumination options, compute-friendly.
- Precedent: Stunt Rally 3 uses OGRE-Next.
- Cons: not a drop-in. Different API (OGRE-Next is a fork/rewrite of the scene system), all RoR rendering code (meshes for flexbodies, deformable wheels, particles, terrain, GUI overlays) must be rewritten. Smaller community.
- Open risk: dynamically deforming meshes ("flexbodies") must be updated every frame from the physics; need to check OGRE-Next's vertex update path for this.

### C. Decouple physics into a library, render in a new engine (high risk, high payoff)
Candidate engines:
- Godot 4 (MIT): license-compatible with GPL, Vulkan, open source community fit. Rendering quality is decent but not top tier.
- Bevy (MIT/Apache): Rust, young, would require C++ <-> Rust bridge.
- Stride (MIT, C#): modern PBR renderer.
- Unreal / Unity: best-looking out of the box, BUT proprietary engines are generally considered incompatible with distributing a GPLv3 project that links or embeds them. Needs legal review (not legal advice). Probably a non-starter for an open-source community fork.
- Work required: extract physics from the game loop into a standalone library with a clean interface (positions of nodes out, controls in), write a loader for the .truck/.load/.terrn2 formats, rebuild terrain, vehicles, UI, multiplayer, scripting bindings.

### D. Hybrid: renderer abstraction layer
- Introduce a thin interface between physics/game logic and rendering, then swap backends over time. Most flexible but takes the longest to pay back.

---

## 5. Working recommendation (Session 1 opinion, not a decision)

1. Start with A. It is where the visible win is cheapest: PBR materials, lighting, post-processing, and 3-5 showcase vehicles + 1-2 showcase maps. Prove the "2026 look" first.
2. In parallel, do a feasibility spike for B or C on one narrow question: can a softbody-deformed mesh be rendered efficiently in the target engine, driven from RoR's physics output?
3. Only commit to a full engine swap if the spike is clean AND there is a contributor team. Otherwise the project risks becoming a rewrite nobody finishes.
4. Treat physics modernization (SIMD/SoA data layout, multithreading, GPU compute) as a separate later track. It is the real path toward BeamNG-level vehicle counts and fidelity, but it conflicts with "keep physics original" unless results are bit-compatible or optional.

---

## 5b. Owner direction (Session 1, later)

Owner wants an upstream-style update of open-source RoR: modern BeamNG-like visuals (PBR), performance optimization, and multiplayer improvements ("all the good stuff"). Not a from-scratch engine port.
Provisional decision: PROCEED WITH OPTION A (stay on OGRE, upgrade to 14.x, PBR materials + modern lighting + post-processing). Engine swap (B/C) is NOT on the critical path; revisit only after a feasibility spike.
Workstreams:
1. Visuals: PBR pipeline, HDR/tonemapping, IBL/skybox, shadows, SSAO/SSR, terrain and vehicle material re-authoring.
2. Performance: profile first. Likely CPU-bound on physics (cache-unfriendly softbody structs, limited threading), so renderer upgrade alone will not fix FPS.
3. Multiplayer: RoRnet 2.45 and ror-server; scope TBD (anti-lag, bandwidth, server tooling, UI).
4. Compatibility: keep existing mods and vehicle formats working.

---

## 5c. Repo findings (Session 1b — cloned master, commit ebf31a2, 2026-10-19)

Location in session workspace: /home/claude/ror-src/rigs-of-rods (shallow clone; re-clone if session resets).

Size:
- ~515 C++ files, ~220k lines in source/ (includes bundled third-party such as imgui).
- source/main/gfx ~40k lines; source/main/physics ~32k lines.
- 195 files include OGRE headers; ~8,650 `Ogre::` references.
- Ogre:: references by area: gfx 3139, physics 879, terrain 735, scripting 719, gui 410, resources 403, utils 168, gameplay 109, audio 106.

Dependencies (conanfile.py / CMake):
- OGRE 1.11.6.1 (fork), components: Bites, Overlay, Paging, RTShaderSystem, MeshLodGenerator, Terrain
- MyGUI 3.4.0, Caelum 0.6.3.1 (sky), AngelScript 2.38, OpenAL Soft 1.24.3
- Also bundled/optional: Hydrax (water), SkyX (sky), PagedGeometry (vegetation), imgui

IMPORTANT CORRECTION to earlier assumption: RoR is on OGRE 1.11 (2019), NOT on 13/14. Step 1 of Option A is therefore a real migration: 1.11 -> 14.x (13 broke/removed some APIs, e.g. Cg program support, and changed namespaces/resource handling).

Shaders/materials (resources/, content/):
- 24 Cg, 19 HLSL, 12 GLSL, 3 GLSL ES, 10 .program, 48 .material, 2 .compositor
- Cg shaders will need porting (Cg plugin is a dependency risk). There is only a small shader base, which is good news for a PBR rewrite.
- Uses RTShader System (rtshader/ folder, used in TerrainObjectManager).

Rendering subsystems found in source/main/gfx (each needs checking during any upgrade):
GfxActor/GfxScene, Hydrax and SkyX/Caelum sky+water managers (multiple legacy addons), ShadowManager, EnvironmentMap (reflections), Skidmarks, DustPool, MovableText, overlays, SurveyMap, particles, camera.

Physics/renderer coupling (nuance to earlier claim "physics is independent of renderer"):
- The solver algorithm is independent, but physics code uses OGRE math types heavily (Vector3 ~409 uses, Quaternion, Real, String) and the spawner/flex code creates Ogre meshes and scene nodes directly.
- Flex classes (source/main/physics/flex: FlexBody, FlexMesh, FlexMeshWheel, FlexObj, FlexAirfoil) mix deformation math with Ogre vertex buffers. They are the critical pieces for any renderer change.
- Practical read: keeping OGRE math types is fine; rendering concerns can be separated at the Flex* + ActorSpawner + GfxActor boundary.

Implication for plan:
- Option A is bigger than assumed (version jump + legacy addons), but still far smaller than a port. The 1.11 -> 14 migration is a prerequisite for modern PBR/Vulkan.
- The small shader base makes a clean PBR pipeline realistic.

Updated next steps:
- [x] Clone repo, record OGRE version
- [x] Size the Ogre:: surface (done at high level)
- [ ] Try building current master (needs Conan + network; session network may block). Otherwise build on owner's machine.
- [ ] List all uses of APIs removed/changed between OGRE 1.11 and 14 (Cg, Terrain, Overlay, resource groups)
- [ ] Decide fate of legacy addons (Hydrax, SkyX, Caelum, PagedGeometry, MyGUI): keep, replace, or drop
- [ ] Spike: minimal PBR material on one vehicle and terrain in the current build

---

## 5d. Build attempt + OGRE 1.11 -> 14 impact scan (Session 1c)

Build:
- This session has cmake, g++, clang++, ninja, conan, but NO OGRE/MyGUI dev libs, and package installs/network are blocked here. So RoR cannot be built in this session.
- BUILDING.md lists deps: OGRE 1.11.6.1, OpenAL >= 1.18, OIS 1.4, MyGUI 3.4.0, fmt, RapidJSON; optional SocketW (network play), AngelScript, CURL, Caelum, PagedGeometry, discord_rpc, OpenSSL, nvidia-cg-toolkit.
- Build is done via Conan (conanfile.py) or the ror-dependencies repo. Must be built on the owner's machine (Windows or Linux) or a CI runner with network.
- Wiki build guide: https://github.com/RigsOfRods/rigs-of-rods/wiki

API usage scan of source/ (counts of `Ogre::X` references):
- Entity 118, SceneManager 64, HardwareVertexBufferSharedPtr 28, Root 20, RenderWindow 20, HardwareBufferManager 16, SubEntity 6, Mesh 6.
- Ogre::Terrain (Terrain component): 6 files (Terrain.cpp, TerrainGeometryManager, ShadowManager, GameScript...). Terrain component still exists in OGRE 14.
- Overlay system: 18 files. MyGUI: 18 files. OIS input: 20 files.
- RTShader System: used only in TerrainObjectManager.cpp; user setting `gfx_enable_rtshaders` defaults to FALSE. So RTSS is effectively not in the default render path.
- Cg: no Cg code in C++ sources; Cg is only an optional non-free dependency for shader effects some mods use. (Cg shaders sit in resources: 24 files.)
- Only two `OGRE_VERSION` checks in code (ContentManager.cpp, TerrainGeometryManager.cpp cache filename). Code already tolerates multiple 1.x versions.

What this means:
- The "v1" entity/mesh/hardware-buffer style API (Entity, SubEntity, HardwareVertexBuffer) is retained in OGRE 14 but REMOVED in OGRE-Next (which uses Item/Hlms/v2 buffers). This confirms: Option A (OGRE 14) is a version bump; Option B (OGRE-Next) is a rewrite of the flexbody/mesh code (about 118 Entity + 28 vertex-buffer sites plus everything in gfx).
- Likely 1.11 -> 14 work items (to verify by compiling against 14.x): ResourceGroupManager/Archive API changes, renamed headers, smart-pointer changes, MaterialPtr/Pass API changes, Terrain component API tweaks, input/window creation (14 prefers SDL via Bites), Overlay changes, removal of deprecated GL/D3D9 render systems. Needs an actual compile pass to produce the real list.
- Legacy renderers in resources: Direct3D9 and fixed-function/Cg-era materials are a clean-up opportunity (move to GL3+/Vulkan/D3D11).

Risk notes:
- Third-party add-ons (Caelum, Hydrax, SkyX, PagedGeometry, MyGUI) are pinned to old OGRE and may not have maintained ports to OGRE 14. Each needs a keep/port/replace decision.
- Resource formats (.material, .program, .compositor) are mod-facing; changing them can break community mods. Plan a compatibility layer or a versioned "modern" material set.

Updated next steps:
- [x] Attempt build (blocked in this session, documented)
- [x] Scan source for OGRE-version-sensitive APIs
- [ ] Owner: build current master locally (Conan) and confirm it runs; report GPU/OS
- [ ] Owner or CI: attempt compile against OGRE 14.x to produce the real break list
- [ ] Inventory add-ons: check maintenance status and OGRE 14 compatibility for Caelum, Hydrax, SkyX, PagedGeometry, MyGUI
- [ ] Define the visual benchmark scene and PBR spike plan

---

## 5e. Owner decisions (Session 1d)

- Visual target: BeamNG-style realism (not stylized). Implies: PBR metal/roughness, HDR + filmic tonemapping, image-based lighting, good sun shadows (cascaded), SSAO, screen-space or probe reflections, atmospheric sky, wet/dirt/damage-aware materials later.
- Build/CI: owner works from a phone, so builds run on GitHub Actions in a fork. Claude writes the workflow and code; owner only taps through GitHub.
- Plan: fork RigsOfRods/rigs-of-rods (GPL-3.0, keep license), e.g. as `<owner>/rigs-of-rods-modern`. Keep this state file in the fork under `docs/modernization/PROJECT_STATE.md` so it travels with the code.
- Branch plan: `modern/ogre14-port` (engine bump), `modern/pbr-pipeline` (materials + lighting), `modern/perf` (profiling/optimization), `modern/mp` (multiplayer).

Blocker found: this session has no GitHub account linked (gh token invalid; list_repos says "no GitHub account linked"). Until the owner connects GitHub in claude.ai settings (Connectors), Claude can only read public repos and cannot push or open PRs.

Update (S1d): Owner does not see a GitHub entry under claude.ai Connectors. Connector registry search for "github" returned no GitHub connector. The link this session needs is the Claude Code GitHub connection (separate from Connectors). Options:
- Path 1: Open this session via claude.ai/code (browser) and use its "connect GitHub" prompt / install the Claude GitHub App on the fork. Exact menu wording unverified.
- Path 2 (no linking): Claude writes files (CI workflow, patches, this state file); owner adds them to the fork via the GitHub web editor ("Add file > Create new file / Upload"), and GitHub Actions runs the build. Slower but works from a phone.

UPDATE (S1e): GitHub linked. Fork exists: https://github.com/moeronald-design/rigs-of-rods-modern (public, fork of RigsOfRods/rigs-of-rods, same commit ebf31a2). Cloned in session at /home/claude/rigs-of-rods-modern (shallow). add_repo succeeded but PUSH IS REFUSED: Claude GitHub App is not installed/authorized on this repo yet. Fix: install the app on the fork at https://github.com/apps/claude/installations/select_target (select account moeronald-design, grant access to rigs-of-rods-modern), or relink at https://claude.ai/customize/connectors?auth_start=github&auth_start_force=1.

Owner to-do (mobile) [original list, superseded by the update above]:
1. claude.ai (Settings > Connectors) > connect GitHub; allow the Claude GitHub App on the fork.
2. In GitHub app/site: open github.com/RigsOfRods/rigs-of-rods > Fork > create fork (name `rigs-of-rods-modern`).
3. Tell Claude the fork's `owner/repo`.

Then Claude: adds the fork to the session, adds the GitHub Actions workflow (Conan build for Windows + Linux, upload artifacts), commits this state file, opens a first PR.

---

## 5f. Upstream AI policy + existing CI (Session 1e)

Upstream AI_POLICY.md (read in the fork) — rules that shape how we work:
- AI-assisted contributions are welcome IF the human contributor understands, reviews, tests, and takes responsibility for them.
- Material AI help MUST be disclosed in the PR (tool + what it did). Undisclosed AI help can get a PR closed.
- Autonomous agent PRs are NOT accepted. An agent must not find work, write it, and open issues/PRs on its own; PRs that look autonomous are closed without review.
- No speculative, bulk, cosmetic, or unrequested refactors. Broad or substantial changes need maintainer agreement BEFORE implementation.
Consequences for this project:
- Work freely in the owner's fork (moeronald-design/rigs-of-rods-modern). Claude commits only to the fork, only on request.
- Do NOT open PRs to RigsOfRods/rigs-of-rods from Claude. Any upstream PR is opened by the owner, who reviews/tests the change, writes the PR text, and discloses AI use.
- Before big changes (OGRE 14 migration, PBR pipeline), open a discussion with maintainers (forum/Discord/GitHub issue written by the owner) to get agreement. A fork can stay a standalone modernization branch if upstream declines.

Existing CI in the fork (inherited from upstream, .github/workflows/):
- build-game.yml: runs on push and pull_request; Ubuntu 22.04 Conan build; uploads `ror-linux` artifact (redist folder). Uses the AnotherFoxGuy Conan remote (nexus.anotherfoxguy.com). Optional itch.io upload only if BUTLER_API_KEY is set (it is not in the fork).
- linux-native.yml: builds OGRE 1.11.6, MyGUI, Caelum, PagedGeometry, SocketW, AngelScript from source on master pushes.
- No new workflow needed to start. Owner must enable Actions on the fork (GitHub shows an "I understand my workflows, enable them" button under the Actions tab).
- No Windows CI job is present in build-game.yml; add later if Windows testing is needed.

---

## 5g. Maintainer feedback (Session 1f)

- Owner reports the RoR maintainers are OK with the modernization effort (reported by owner, no link or quote recorded yet). TODO: owner to save where/when they said so (forum thread, Discord, or GitHub issue URL) so it can be cited in future PR descriptions.
- Still required by AI_POLICY: owner reviews and tests each change, discloses AI help in PRs, and opens any upstream PR personally. Per-feature agreement (OGRE 14 bump, PBR material format) should be confirmed in writing before large PRs.
- Baseline CI check (public GitHub API, Session 1f): 0 workflow runs on the fork, so Actions is not enabled yet or no run has been triggered. Next push after enabling should start `build-game.yml`.

---

## 6. Open questions for the owner

- What is "success"? Screenshots that look modern, or a fully new engine port?
- Solo project, or recruiting a community team? (Determines whether B/C are realistic.)
- Is staying compatible with existing RoR mods and multiplayer servers required?
- Should this be a fork or upstream contributions to RigsOfRods/rigs-of-rods?
- Which platforms: Windows only, or Windows + Linux (RoR supports both; OGRE-Next adds Metal for macOS)?
- Budget for assets: who makes PBR vehicles and terrains?

---

## 7. Next steps (suggested)

- [ ] Clone repo, record exact OGRE version and render-system plugins in use
- [ ] Map every file that touches Ogre:: (size the porting surface)
- [ ] Catalogue rendering features: flexbodies, deformable wheels, terrain (OTC?), water (Hydrax?), sky, particles, shadows, GUI
- [ ] Read developer docs: Codebase overview, physics module, Ogre bindings
- [ ] Read the forum port-checklist thread in a normal browser (blocked for me) and paste into the next chat
- [ ] Decide on A vs B vs C
- [ ] Define a visual benchmark: one vehicle, one map, same camera, RoR today vs target

---

## 8. Sources

- Rigs of Rods GitHub org: https://github.com/rigsofrods
- Developer portal / codebase overview: https://developer.rigsofrods.org/d4/d38/_codebase_overview_page.html
- Physics module docs: https://developer.rigsofrods.org/d5/d74/group___physics.html
- Forum, "Enter the Codebase": https://forum.rigsofrods.org/threads/enter-the-codebase.121/
- Forum, port-checklist thread (search snippet only; page blocked): https://forum.rigsofrods.org/threads/checklist-for-porting-to-different-3d-engine-than-ogre.3510/
- Forum, 2026.01 release: https://forum.rigsofrods.org/threads/rigs-of-rods-2026-01-released.4887/
- Wikipedia mirror (engine, license, stable release): https://wiki.krisyotam.com/content/wikipedia_en_all_nopic_2026-03/Rigs_of_Rods
- OGRE downloads (14.5.2): https://www.ogre3d.org/download/sdk/sdk-ogre
- OGRE-Next 3.0 release: https://www.ogre3d.org/2024/10/16/ogre-next-3-0-0-eris-released
- OGRE-Next 4.0 changes: https://ogrecave.github.io/ogre-next/api/latest/_ogre40_changes.html
- Phoronix on OGRE-Next 3.0 / Stunt Rally 3: https://www.phoronix.com/news/OGRE-Next-3.0

---

## 9. Handoff prompt (paste into the next chat)

> Continuing the Rigs of Rods modernization project. Read the attached RoR_Modernization_Project_State.md first. Summary: RoR is GPL-3.0, C++/AngelScript, renders with OGRE (not a game engine), and uses its own in-house node/beam soft-body physics, so the renderer can be changed without touching physics. Current plan is Option A (upgrade OGRE + PBR/lighting/assets) with a feasibility spike on OGRE-Next or a Godot-based renderer. Next task: [fill in — e.g. size the Ogre:: porting surface from the repo / draft the visual benchmark / spec the physics-to-renderer interface]. Update the state file at the end of the session.
