/**
 * Pokémon KaizoRogue — Modern Web Meta-Hub & Local Runner
 * FireRed PC Box Roster, Regional Gacha Machine, Authentic Egg Animations, & In-Memory Prerun Injection
 */

(function () {
    'use strict';

    // Nature definitions
    const NATURES = [
        "Hardy", "Lonely", "Brave", "Adamant", "Naughty",
        "Bold", "Docile", "Relaxed", "Impish", "Lax",
        "Timid", "Hasty", "Serious", "Jolly", "Naive",
        "Modest", "Mild", "Quiet", "Bashful", "Rash",
        "Calm", "Gentle", "Sassy", "Careful", "Quirky"
    ];

    // Generations metadata
    const GENERATIONS = [
        { gen: 1, name: "Kanto", start: 1, end: 151, pill: "🔴 Kanto" },
        { gen: 2, name: "Johto", start: 152, end: 251, pill: "⚪ Johto" },
        { gen: 3, name: "Hoenn", start: 252, end: 386, pill: "🟢 Hoenn" },
        { gen: 4, name: "Sinnoh", start: 387, end: 493, pill: "🔵 Sinnoh" },
        { gen: 5, name: "Unova", start: 494, end: 649, pill: "⚫ Unova" },
        { gen: 6, name: "Kalos", start: 650, end: 721, pill: "🌸 Kalos" },
        { gen: 7, name: "Alola", start: 722, end: 809, pill: "🌺 Alola" },
        { gen: 8, name: "Galar", start: 810, end: 905, pill: "⚔️ Galar" },
        { gen: 9, name: "Paldea", start: 906, end: 1025, pill: "🍇 Paldea" }
    ];

    const BANNER_NAMES = [
        { id: 0, title: "ALL GENERATIONS", desc: "Include tutti i Pokémon da Kanto a Paldea (Gen 1 - 9)." },
        { id: 1, title: "GEN 1: KANTO", desc: "Estrazione speciale riservata ai 151 Pokémon di Kanto (#001 - #151)." },
        { id: 2, title: "GEN 2: JOHTO", desc: "Estrazione speciale riservata ai 100 Pokémon di Johto (#152 - #251)." },
        { id: 3, title: "GEN 3: HOENN", desc: "Estrazione speciale riservata ai 135 Pokémon di Hoenn (#252 - #386)." },
        { id: 4, title: "GEN 4: SINNOH", desc: "Estrazione speciale riservata ai 107 Pokémon di Sinnoh (#387 - #493)." },
        { id: 5, title: "GEN 5: UNOVA", desc: "Estrazione speciale riservata ai 156 Pokémon di Unova (#494 - #649)." },
        { id: 6, title: "GEN 6: KALOS", desc: "Estrazione speciale riservata ai 72 Pokémon di Kalos (#650 - #721)." },
        { id: 7, title: "GEN 7: ALOLA", desc: "Estrazione speciale riservata agli 88 Pokémon di Alola (#722 - #809)." },
        { id: 8, title: "GEN 8: GALAR", desc: "Estrazione speciale riservata ai 96 Pokémon di Galar (#810 - #905)." },
        { id: 9, title: "GEN 9: PALDEA", desc: "Estrazione speciale riservata ai Pokémon di Paldea (#906 - #1025)." }
    ];

    // Shop item definitions
    const SHOP_ITEMS = [
        { id: 462, name: "Rapidoartiglio (Quick Claw)", desc: "Può far attaccare per primo a inizio turno.", price: 500, icon: "⚡" },
        { id: 481, name: "Focalnastro (Focus Sash)", desc: "Resiste a un colpo letale da HP massimi a 1 HP.", price: 800, icon: "🎗️" },
        { id: 442, name: "Bendascelta (Choice Band)", desc: "Aumenta l'Attacco del 50%, blocca su una sola mossa.", price: 1000, icon: "🥊" },
        { id: 443, name: "Lentiscelta (Choice Specs)", desc: "Aumenta l'Att. Sp. del 50%, blocca su una mossa.", price: 1000, icon: "👓" },
        { id: 444, name: "Stolascelta (Choice Scarf)", desc: "Aumenta la Velocità del 50%, blocca su una mossa.", price: 1000, icon: "🧣" },
        { id: 472, name: "Avanzi (Leftovers)", desc: "Recupera 1/16 dei PS massimi a ogni turno.", price: 800, icon: "🍏" },
        { id: 522, name: "Baccaprugna (Lum Berry)", desc: "Cura qualsiasi alterazione di stato.", price: 200, icon: "🫐" },
        { id: 523, name: "Baccacedro (Sitrus Berry)", desc: "Recupera il 25% dei PS massimi a metà salute.", price: 200, icon: "🍊" },
        { id: 79,  name: "Capsula Abilità Custom", desc: "Permette di cambiare casualmente l'abilità del Pokémon.", price: 600, icon: "💊" }
    ];

    const DUP_POINTS = {
        common: 5,
        uncommon: 10,
        rare: 15,
        epic: 25,
        legendary: 50
    };

    // State
    const state = {
        meta: null,
        pokedex: [],
        account: null,
        baseRomBuffer: null,
        kaizoRomBuffer: null,
        kaizoRomBlob: null,
        kaizoRomUrl: null,
        patchReady: false,
        // UI Navigation State
        selectedPcGen: 1, // Kanto
        selectedPcMonId: 1, // Bulbasaur
        selectedGachaBanner: 0 // All
    };

    // Helper: random IV generator (0 to 31)
    function generateRandomIvs() {
        return {
            hp: Math.floor(Math.random() * 32),
            atk: Math.floor(Math.random() * 32),
            def: Math.floor(Math.random() * 32),
            speed: Math.floor(Math.random() * 32),
            spAtk: Math.floor(Math.random() * 32),
            spDef: Math.floor(Math.random() * 32)
        };
    }

    // =========================================================================
    // Account Management
    // =========================================================================
    function getDefaultAccount() {
        return {
            trainerName: "Allenatore Kaizo",
            runsCount: 0,
            runsWon: 0,
            surgeWinsCount: 0,
            shards: 250,
            tickets: 5,
            money: 3000,
            rarityPoints: {
                common: 25,
                uncommon: 15,
                rare: 10,
                epic: 0,
                legendary: 0
            },
            roster: [
                {
                    id: 1,
                    key: "BULBASAUR",
                    name: "Bulbasaur",
                    tier: "common",
                    nature: 0, // Hardy
                    abilityNum: 0,
                    isShiny: false,
                    shinyUnlocked: false,
                    beatSurge: false,
                    megaUnlocked: [],
                    ivs: { hp: 20, atk: 18, def: 24, speed: 15, spAtk: 26, spDef: 22 },
                    happiness: 255
                },
                {
                    id: 4,
                    key: "CHARMANDER",
                    name: "Charmander",
                    tier: "common",
                    nature: 3, // Adamant
                    abilityNum: 0,
                    isShiny: false,
                    shinyUnlocked: false,
                    beatSurge: false,
                    megaUnlocked: [],
                    ivs: { hp: 16, atk: 28, def: 14, speed: 25, spAtk: 19, spDef: 15 },
                    happiness: 255
                },
                {
                    id: 7,
                    key: "SQUIRTLE",
                    name: "Squirtle",
                    tier: "common",
                    nature: 15, // Modest
                    abilityNum: 0,
                    isShiny: false,
                    shinyUnlocked: false,
                    beatSurge: false,
                    megaUnlocked: [],
                    ivs: { hp: 22, atk: 14, def: 27, speed: 18, spAtk: 24, spDef: 25 },
                    happiness: 255
                }
            ],
            activeStarterId: 1,
            purchasedItems: { 462: 1, 522: 2 },
            equippedBagItems: [462] // Quick Claw by default
        };
    }

    function loadAccount() {
        try {
            const raw = localStorage.getItem('kaizo_account_v2');
            if (raw) {
                state.account = JSON.parse(raw);
                return;
            }
            // Migration from v1 if present
            const old = localStorage.getItem('kaizo_account_v1');
            if (old) {
                state.account = JSON.parse(old);
                // Ensure IVs are objects not all 31
                state.account.roster.forEach(m => {
                    if (!m.ivs || typeof m.ivs.hp !== 'number') m.ivs = generateRandomIvs();
                });
                saveAccount();
                return;
            }
        } catch (e) {
            console.warn('Errore lettura account:', e);
        }
        state.account = getDefaultAccount();
        saveAccount();
    }

    function saveAccount() {
        try {
            localStorage.setItem('kaizo_account_v2', JSON.stringify(state.account));
            updateWalletUI();
        } catch (e) {
            console.error('Errore salvataggio account:', e);
        }
    }

    // =========================================================================
    // CRC32 Calculation
    // =========================================================================
    const crcTable = new Uint32Array(256);
    for (let i = 0; i < 256; i++) {
        let c = i;
        for (let k = 0; k < 8; k++) {
            c = (c & 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1);
        }
        crcTable[i] = c >>> 0;
    }

    function calculateCrc32(uint8Array) {
        let crc = 0 ^ (-1);
        const len = uint8Array.length;
        for (let i = 0; i < len; i++) {
            crc = (crc >>> 8) ^ crcTable[(crc ^ uint8Array[i]) & 0xFF];
        }
        return ((crc ^ (-1)) >>> 0).toString(16).padStart(8, '0');
    }

    function getSpriteUrl(dexId, isShiny) {
        if (!dexId) return '/assets/egg_clean.png';
        return `/assets/sprites/front${isShiny ? '_shiny' : ''}/${dexId}.png`;
    }

    function getIconUrl(dexId) {
        if (!dexId) return '/assets/egg_icon_clean.png';
        return `/assets/sprites/icon/${dexId}.png`;
    }

    function getEggSpriteUrl(tier, isShiny, frame = 'intact') {
        const t = (tier || 'common').toLowerCase();
        const shinyPrefix = isShiny ? 'shiny_' : '';
        if (frame === 'hatched' || frame === 3) {
            return `/assets/egg_hatched_${shinyPrefix}${t}.png`;
        }
        if (frame === 'crack1' || frame === 1) {
            return `/assets/egg_crack1_${shinyPrefix}${t}.png`;
        }
        if (frame === 'crack2' || frame === 2) {
            return `/assets/egg_crack2_${shinyPrefix}${t}.png`;
        }
        return `/assets/egg_${shinyPrefix}${t}.png`;
    }

    // =========================================================================
    // IV Grades & Radar Graph Generator (Authentic ROM Style)
    // =========================================================================
    function getIvGrade(val) {
        const v = typeof val === 'number' ? val : 0;
        if (v >= 31) return { grade: 'S+', color: '#f8c838', bg: '#3a2e0a' }; // Gold
        if (v >= 30) return { grade: 'S',  color: '#c084fc', bg: '#2f1544' }; // Purple
        if (v >= 25) return { grade: 'A',  color: '#ef4444', bg: '#3f1313' }; // Red
        if (v >= 18) return { grade: 'B',  color: '#3b82f6', bg: '#132844' }; // Blue
        if (v >= 10) return { grade: 'C',  color: '#22c55e', bg: '#103418' }; // Green
        return { grade: 'D',  color: '#94a3b8', bg: '#1e2430' }; // Gray
    }

    function getTotalIvGrade(tot) {
        const t = typeof tot === 'number' ? tot : 0;
        if (t >= 186) return { grade: 'S+', color: '#f8c838', bg: '#3a2e0a' };
        if (t >= 165) return { grade: 'S',  color: '#c084fc', bg: '#2f1544' };
        if (t >= 140) return { grade: 'A',  color: '#ef4444', bg: '#3f1313' };
        if (t >= 110) return { grade: 'B',  color: '#3b82f6', bg: '#132844' };
        if (t >= 75)  return { grade: 'C',  color: '#22c55e', bg: '#103418' };
        return { grade: 'D',  color: '#94a3b8', bg: '#1e2430' };
    }

    function renderIvRadarGraphSvg(ivs) {
        if (!ivs) ivs = { hp: 15, atk: 15, def: 15, speed: 15, spDef: 15, spAtk: 15 };
        const cx = 130;
        const cy = 105;
        const R = 62;
        const minR = 6;

        // 6 Vertices in ROM order: HP, ATK, DEF, SPE, SPD, SPA
        const angles = [
            -Math.PI / 2,      // 0: HP (top)
            -Math.PI / 6,      // 1: ATK (top-right)
            Math.PI / 6,       // 2: DEF (bottom-right)
            Math.PI / 2,       // 3: SPE (bottom)
            5 * Math.PI / 6,   // 4: SPD (bottom-left)
            -5 * Math.PI / 6   // 5: SPA (top-left)
        ];

        const stats = [
            { key: 'hp', name: 'HP', val: ivs.hp || 0 },
            { key: 'atk', name: 'ATK', val: ivs.atk || 0 },
            { key: 'def', name: 'DEF', val: ivs.def || 0 },
            { key: 'speed', name: 'SPE', val: ivs.speed || 0 },
            { key: 'spDef', name: 'SPD', val: ivs.spDef || 0 },
            { key: 'spAtk', name: 'SPA', val: ivs.spAtk || 0 }
        ];

        const maxPoints = angles.map(a => `${cx + R * Math.cos(a)},${cy + R * Math.sin(a)}`).join(' ');
        const midR = R / 2;
        const midPoints = angles.map(a => `${cx + midR * Math.cos(a)},${cy + midR * Math.sin(a)}`).join(' ');
        const qR = R / 4;
        const qPoints = angles.map(a => `${cx + qR * Math.cos(a)},${cy + qR * Math.sin(a)}`).join(' ');

        const valPoints = stats.map((s, i) => {
            const r = minR + (Math.min(31, Math.max(0, s.val)) / 31) * (R - minR);
            const a = angles[i];
            return `${cx + r * Math.cos(a)},${cy + r * Math.sin(a)}`;
        }).join(' ');

        const axisLines = angles.map(a => {
            const x2 = cx + R * Math.cos(a);
            const y2 = cy + R * Math.sin(a);
            return `<line x1="${cx}" y1="${cy}" x2="${x2}" y2="${y2}" stroke="#28374a" stroke-width="1.5" />`;
        }).join('');

        const dots = angles.map(a => {
            const x = cx + R * Math.cos(a);
            const y = cy + R * Math.sin(a);
            return `<circle cx="${x}" cy="${y}" r="2.5" fill="#ffffff" />`;
        }).join('');

        const labelDefs = [
            { x: 130, y: 22, anchor: 'middle' },
            { x: 200, y: 70, anchor: 'start' },
            { x: 200, y: 144, anchor: 'start' },
            { x: 130, y: 196, anchor: 'middle' },
            { x: 60, y: 144, anchor: 'end' },
            { x: 60, y: 70, anchor: 'end' }
        ];

        const labels = stats.map((s, i) => {
            const pos = labelDefs[i];
            const gr = getIvGrade(s.val);
            return `
                <text x="${pos.x}" y="${pos.y}" text-anchor="${pos.anchor}" font-family="'Press Start 2P', monospace" font-size="8" fill="#cbd5e1">
                    ${s.name} <tspan fill="${gr.color}" font-weight="bold">${gr.grade}</tspan>
                </text>
            `;
        }).join('');

        const totalIv = stats.reduce((acc, cur) => acc + cur.val, 0);
        const totGr = getTotalIvGrade(totalIv);

        return `
            <svg viewBox="0 0 260 205" class="iv-radar-svg" xmlns="http://www.w3.org/2000/svg">
                <!-- Background Guides (Hexagon + Mid Axis) -->
                <polygon points="${maxPoints}" fill="#0a0f17" stroke="#33465e" stroke-width="1.5" />
                <polygon points="${midPoints}" fill="none" stroke="#223044" stroke-width="1" stroke-dasharray="3 3" />
                <polygon points="${qPoints}" fill="none" stroke="#16202e" stroke-width="1" stroke-dasharray="2 2" />
                ${axisLines}
                ${dots}

                <!-- Shaded Cyan Polygon (Game Celeste Style) -->
                <polygon points="${valPoints}" fill="rgba(56, 189, 248, 0.45)" stroke="#ffffff" stroke-width="2" />

                <!-- Center Total Grade Watermark -->
                <text x="${cx}" y="${cy + 5}" text-anchor="middle" font-family="'Press Start 2P', monospace" font-size="10" fill="rgba(255,255,255,0.2)">
                    ${totGr.grade}
                </text>

                <!-- 6 Vertex Stat Labels with Colored Rank Letters -->
                ${labels}
            </svg>
        `;
    }

    // =========================================================================
    // Initialization
    // =========================================================================
    async function init() {
        loadAccount();
        initTabs();
        initDropzone();
        initAccountActions();
        initDevMode();
        initFireRedPcControls();
        initGachaMachine();
        initMarketShop();
        initPlayRunner();
        initSaveManager();

        await Promise.all([
            loadMetadata(),
            loadPokedex()
        ]);

        renderProfileAndHub();
        renderFireRedPc();
        renderMarket();
        renderPrerunSummary();
    }

    async function loadMetadata() {
        try {
            const res = await fetch('patch/meta.json');
            if (res.ok) state.meta = await res.json();
        } catch (e) {
            console.warn('Impossibile caricare meta.json:', e);
        }
    }

    async function loadPokedex() {
        try {
            const res = await fetch('data/pokedex.json');
            if (res.ok) {
                state.pokedex = await res.json();
                console.log(`[KaizoRogue] Pokédex caricato: ${state.pokedex.length} Pokémon.`);
            }
        } catch (e) {
            console.warn('Impossibile caricare data/pokedex.json:', e);
        }
    }

    function initTabs() {
        document.querySelectorAll('.nav-btn, .nav-trigger').forEach(btn => {
            btn.addEventListener('click', () => {
                const targetTab = btn.getAttribute('data-tab');
                if (!targetTab) return;

                document.querySelectorAll('.nav-btn').forEach(b => b.classList.remove('active'));
                document.querySelectorAll('.tab-panel').forEach(p => p.classList.remove('active'));

                const matchingNav = document.querySelector(`.nav-btn[data-tab="${targetTab}"]`);
                if (matchingNav) matchingNav.classList.add('active');

                const targetPanel = document.getElementById(`tab-${targetTab}`);
                if (targetPanel) targetPanel.classList.add('active');

                if (targetTab === 'roster') renderFireRedPc();
                if (targetTab === 'market') renderMarket();
                if (targetTab === 'play') renderPrerunSummary();
            });
        });
    }

    function updateWalletUI() {
        const acc = state.account;
        if (!acc) return;

        const isDev = !!acc.devMode;

        const setTxt = (id, val) => {
            const el = document.getElementById(id);
            if (el) el.textContent = typeof val === 'number' ? val.toLocaleString() : val;
        };

        setTxt('wallet-shards', isDev ? '∞' : acc.shards);
        setTxt('wallet-tickets', isDev ? '∞' : acc.tickets);
        setTxt('wallet-money', isDev ? '₽ ∞' : `₽ ${(acc.money || 0).toLocaleString()}`);
        setTxt('nav-roster-count', acc.roster.length);
        setTxt('gacha-tickets-count', isDev ? '∞ (Dev)' : acc.tickets);

        setTxt('pts-common', isDev ? '∞' : acc.rarityPoints.common);
        setTxt('pts-uncommon', isDev ? '∞' : acc.rarityPoints.uncommon);
        setTxt('pts-rare', isDev ? '∞' : acc.rarityPoints.rare);
        setTxt('pts-epic', isDev ? '∞' : acc.rarityPoints.epic);
        setTxt('pts-legendary', isDev ? '∞' : acc.rarityPoints.legendary);

        setTxt('stat-runs', acc.runsCount);
        setTxt('stat-wins', acc.runsWon);
        setTxt('stat-surge', acc.surgeWinsCount);
        setTxt('stat-roster', acc.roster.length);

        // Header dev badge
        const headerBadge = document.getElementById('header-dev-badge');
        const headerBtn = document.getElementById('btn-toggle-dev-header');
        if (headerBadge) headerBadge.textContent = isDev ? 'DEV: ON' : 'DEV: OFF';
        if (headerBtn) {
            if (isDev) headerBtn.classList.add('active');
            else headerBtn.classList.remove('active');
        }

        // Hub dev toggle
        const devToggle = document.getElementById('dev-mode-toggle');
        const devTxt = document.getElementById('dev-switch-text');
        if (devToggle) devToggle.checked = isDev;
        if (devTxt) devTxt.textContent = isDev ? 'DEV MODE: ATTIVA' : 'DEV MODE: DISATTIVA';
    }

    // =========================================================================
    // Tab 1: Hub & Profile Card
    // =========================================================================
    function renderProfileAndHub() {
        updateWalletUI();
        const acc = state.account;
        const nameEl = document.getElementById('profile-name');
        if (nameEl) nameEl.textContent = acc.trainerName;

        const activeMon = acc.roster.find(m => m.id === acc.activeStarterId) || acc.roster[0];
        if (activeMon) {
            const img = document.getElementById('hub-starter-img');
            const name = document.getElementById('hub-starter-name');
            const details = document.getElementById('hub-starter-details');
            const star = document.getElementById('hub-starter-star');

            if (img) img.src = getSpriteUrl(activeMon.id, activeMon.isShiny);
            if (name) name.textContent = activeMon.name;
            if (details) {
                const natName = NATURES[activeMon.nature] || "Hardy";
                const ivSum = activeMon.ivs ? (activeMon.ivs.hp + activeMon.ivs.atk + activeMon.ivs.def + activeMon.ivs.spAtk + activeMon.ivs.spDef + activeMon.ivs.speed) : 186;
                const totGr = getTotalIvGrade(ivSum);
                details.innerHTML = `Natura: <strong>${natName}</strong> | Totale IV: <strong style="color:${totGr.color}">${totGr.grade}</strong> (${ivSum}/186) | Felicità: <strong>255 MAX</strong>`;
            }
            if (star) {
                if (activeMon.shinyUnlocked) star.classList.remove('hidden');
                else star.classList.add('hidden');
            }
        }
    }

    function initAccountActions() {
        const btnEdit = document.getElementById('btn-edit-name');
        if (btnEdit) {
            btnEdit.addEventListener('click', () => {
                const newName = prompt('Inserisci il nome del tuo Allenatore:', state.account.trainerName);
                if (newName && newName.trim()) {
                    state.account.trainerName = newName.trim().slice(0, 12);
                    saveAccount();
                    renderProfileAndHub();
                }
            });
        }

        const exportHandler = () => {
            const dataStr = "data:text/json;charset=utf-8," + encodeURIComponent(JSON.stringify(state.account, null, 2));
            const a = document.createElement('a');
            a.href = dataStr;
            a.download = `KaizoRogue_Account_${Date.now()}.json`;
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
        };

        document.getElementById('btn-export-account')?.addEventListener('click', exportHandler);
        document.getElementById('btn-export-account-tab')?.addEventListener('click', exportHandler);

        const btnImp = document.getElementById('btn-import-account');
        const inputImp = document.getElementById('account-import-input');
        if (btnImp && inputImp) {
            btnImp.addEventListener('click', () => inputImp.click());
            inputImp.addEventListener('change', async (e) => {
                const file = e.target.files && e.target.files[0];
                if (!file) return;
                try {
                    const text = await file.text();
                    const imported = JSON.parse(text);
                    if (imported && imported.roster) {
                        state.account = imported;
                        saveAccount();
                        renderProfileAndHub();
                        renderFireRedPc();
                        alert('Profilo Account importato con successo!');
                    }
                } catch (err) {
                    alert('File profilo non valido: ' + err.message);
                }
            });
        }

        document.getElementById('btn-reset-account')?.addEventListener('click', () => {
            if (confirm('Sei sicuro di voler resettare il tuo profilo? Tutti i progressi e i Pokémon verranno azzerati.')) {
                state.account = getDefaultAccount();
                saveAccount();
                renderProfileAndHub();
                renderFireRedPc();
            }
        });
    }

    // =========================================================================
    // Dev Mode & Cheats Console
    // =========================================================================
    function initDevMode() {
        const toggle = document.getElementById('dev-mode-toggle');
        const headerBtn = document.getElementById('btn-toggle-dev-header');

        const switchDev = (enable) => {
            state.account.devMode = enable;
            if (enable) {
                // Ensure plenty of resources
                state.account.money = Math.max(state.account.money || 0, 999999);
                state.account.tickets = Math.max(state.account.tickets || 0, 999);
                state.account.shards = Math.max(state.account.shards || 0, 99999);
                ['common', 'uncommon', 'rare', 'epic', 'legendary'].forEach(t => {
                    state.account.rarityPoints[t] = Math.max(state.account.rarityPoints[t] || 0, 9999);
                });
            }
            saveAccount();
            updateWalletUI();
            renderProfileAndHub();
            renderMarket();
        };

        if (toggle) {
            toggle.addEventListener('change', (e) => switchDev(e.target.checked));
        }

        if (headerBtn) {
            headerBtn.addEventListener('click', () => switchDev(!state.account.devMode));
        }

        document.getElementById('dev-btn-infinite-money')?.addEventListener('click', () => {
            state.account.money = (state.account.money || 0) + 999999;
            saveAccount();
            updateWalletUI();
            renderMarket();
            alert('💰 +999.999 Soldi Run aggiunti!');
        });

        document.getElementById('dev-btn-infinite-tickets')?.addEventListener('click', () => {
            state.account.tickets = (state.account.tickets || 0) + 999;
            saveAccount();
            updateWalletUI();
            alert('🎟️ +999 Biglietti Gacha aggiunti!');
        });

        document.getElementById('dev-btn-infinite-shards')?.addEventListener('click', () => {
            state.account.shards = (state.account.shards || 0) + 99999;
            saveAccount();
            updateWalletUI();
            alert('💎 +99.999 Frammenti Rogue aggiunti!');
        });

        document.getElementById('dev-btn-infinite-points')?.addEventListener('click', () => {
            ['common', 'uncommon', 'rare', 'epic', 'legendary'].forEach(t => {
                state.account.rarityPoints[t] = (state.account.rarityPoints[t] || 0) + 9999;
            });
            saveAccount();
            updateWalletUI();
            alert('⭐ +9.999 Punti Rarità aggiunti a tutte le categorie!');
        });

        document.getElementById('dev-btn-unlock-all-mons')?.addEventListener('click', () => {
            let added = 0;
            const existingIds = new Set(state.account.roster.map(m => m.id));
            state.pokedex.forEach(dexEntry => {
                if (!existingIds.has(dexEntry.id)) {
                    state.account.roster.push({
                        id: dexEntry.id,
                        key: dexEntry.key || dexEntry.name.toUpperCase(),
                        name: dexEntry.name,
                        tier: dexEntry.tier,
                        nature: Math.floor(Math.random() * 25),
                        abilityNum: 0,
                        isShiny: false,
                        shinyUnlocked: false,
                        beatSurge: false,
                        megaUnlocked: [],
                        ivs: generateRandomIvs(),
                        happiness: 255
                    });
                    added++;
                }
            });
            state.account.roster.sort((a, b) => a.id - b.id);
            saveAccount();
            renderProfileAndHub();
            renderFireRedPc();
            alert(`🔓 Sbloccati ${added} nuovi Pokémon! Roster completo a ${state.account.roster.length} Pokémon!`);
        });

        document.getElementById('dev-btn-unlock-all-megas')?.addEventListener('click', () => {
            state.account.roster.forEach(m => {
                m.beatSurge = true;
                const dex = state.pokedex.find(p => p.id === m.id);
                if (dex && dex.megaStones) {
                    m.megaUnlocked = dex.megaStones.map(s => s.id);
                }
            });
            saveAccount();
            renderFireRedPc();
            alert('⚡ Tutte le Megapietre sbloccate e Surge Beaten impostato su tutti i Pokémon!');
        });

        document.getElementById('dev-btn-all-shiny')?.addEventListener('click', () => {
            state.account.roster.forEach(m => {
                m.shinyUnlocked = true;
                m.isShiny = true;
            });
            saveAccount();
            renderFireRedPc();
            renderProfileAndHub();
            alert('✨ Tutti i Pokémon del tuo Roster sono ora Shiny con stellina rossa ★!');
        });
    }

    // =========================================================================
    // Tab 2: FireRed PC Box Roster System
    // =========================================================================
    function initFireRedPcControls() {
        // Prev/Next box buttons
        document.getElementById('btn-pc-prev-box')?.addEventListener('click', () => {
            state.selectedPcGen = (state.selectedPcGen > 1) ? state.selectedPcGen - 1 : 9;
            renderFireRedPc();
        });

        document.getElementById('btn-pc-next-box')?.addEventListener('click', () => {
            state.selectedPcGen = (state.selectedPcGen < 9) ? state.selectedPcGen + 1 : 1;
            renderFireRedPc();
        });

        // Region Pills
        document.querySelectorAll('.pc-pill').forEach(pill => {
            pill.addEventListener('click', () => {
                const gen = parseInt(pill.getAttribute('data-gen'), 10);
                if (gen) {
                    state.selectedPcGen = gen;
                    renderFireRedPc();
                }
            });
        });

        // Search in PC
        document.getElementById('pc-search')?.addEventListener('input', () => {
            renderFireRedPc();
        });

        // Summary Actions
        document.getElementById('pc-shiny-checkbox')?.addEventListener('change', (e) => {
            const mon = state.account.roster.find(m => m.id === state.selectedPcMonId);
            if (!mon || !mon.shinyUnlocked) return;
            mon.isShiny = e.target.checked;
            saveAccount();
            renderFireRedPc();
            renderProfileAndHub();
        });

        document.getElementById('btn-pc-reroll-nature')?.addEventListener('click', () => {
            const mon = state.account.roster.find(m => m.id === state.selectedPcMonId);
            if (!mon) return;

            const cost = 5;
            if (!state.account.devMode) {
                if ((state.account.rarityPoints[mon.tier] || 0) < cost) {
                    alert(`Punti ${mon.tier.toUpperCase()} insufficienti (Richiesti: ${cost}).`);
                    return;
                }
                state.account.rarityPoints[mon.tier] -= cost;
            }

            let newNat;
            do {
                newNat = Math.floor(Math.random() * 25);
            } while (newNat === mon.nature);

            mon.nature = newNat;
            saveAccount();
            renderFireRedPc();
            renderProfileAndHub();
        });

        document.getElementById('btn-pc-cycle-ability')?.addEventListener('click', () => {
            const mon = state.account.roster.find(m => m.id === state.selectedPcMonId);
            if (!mon) return;
            mon.abilityNum = (mon.abilityNum === 0) ? 1 : 0;
            saveAccount();
            renderFireRedPc();
        });

        document.getElementById('btn-pc-set-starter')?.addEventListener('click', () => {
            state.account.activeStarterId = state.selectedPcMonId;
            saveAccount();
            renderFireRedPc();
            renderProfileAndHub();
            renderPrerunSummary();
        });
    }

    function renderFireRedPc() {
        const curGenMeta = GENERATIONS.find(g => g.gen === state.selectedPcGen) || GENERATIONS[0];
        const boxTitle = document.getElementById('pc-box-title');
        const boxCounter = document.getElementById('pc-box-counter');
        const slotsGrid = document.getElementById('pc-slots-grid');

        if (boxTitle) boxTitle.textContent = `BOX ${curGenMeta.gen}: ${curGenMeta.name.toUpperCase()}`;

        // Update active pill
        document.querySelectorAll('.pc-pill').forEach(pill => {
            const g = parseInt(pill.getAttribute('data-gen'), 10);
            if (g === state.selectedPcGen) pill.classList.add('active');
            else pill.classList.remove('active');
        });

        // Filter Pokémon belonging to this region
        const genPokedex = state.pokedex.filter(p => p.gen === state.selectedPcGen);
        const unlockedInGen = state.account.roster.filter(m => m.id >= curGenMeta.start && m.id <= curGenMeta.end);

        if (boxCounter) {
            boxCounter.textContent = `Sbloccati: ${unlockedInGen.length} / ${genPokedex.length} (${Math.round((unlockedInGen.length / (genPokedex.length || 1)) * 100)}%)`;
        }

        const searchVal = (document.getElementById('pc-search')?.value || '').toLowerCase();

        // Render Predefined Grid in National Dex Order
        if (slotsGrid) {
            slotsGrid.innerHTML = genPokedex.map(dexEntry => {
                const mon = state.account.roster.find(m => m.id === dexEntry.id);
                const isUnlocked = !!mon;

                if (searchVal) {
                    if (isUnlocked && !mon.name.toLowerCase().includes(searchVal) && !String(mon.id).includes(searchVal)) return '';
                    if (!isUnlocked && !dexEntry.name.toLowerCase().includes(searchVal) && !String(dexEntry.id).includes(searchVal)) return '';
                }

                const isSelected = (dexEntry.id === state.selectedPcMonId);
                const isStarter = isUnlocked && (mon.id === state.account.activeStarterId);
                const numStr = `#${String(dexEntry.id).padStart(3, '0')}`;

                if (isUnlocked) {
                    return `
                        <div class="pc-slot-box ${isSelected ? 'selected' : ''} ${isStarter ? 'is-starter' : ''}" data-id="${mon.id}">
                            ${mon.shinyUnlocked ? `<span class="shiny-star-indicator">★</span>` : ''}
                            <div class="pc-slot-icon-wrap">
                                <img class="pc-slot-img" src="${getIconUrl(mon.id)}" alt="${mon.name}" onerror="this.onerror=null; this.src='/assets/sprites/icon/1.png'" loading="lazy" />
                            </div>
                            <span class="pc-slot-num">${numStr}</span>
                            <span class="pc-slot-name">${mon.name}</span>
                        </div>
                    `;
                } else {
                    return `
                        <div class="pc-slot-box locked" title="Non ancora sbloccato nel Roster">
                            <div class="pc-slot-icon-wrap">
                                <img class="pc-slot-img locked-icon" src="${getIconUrl(dexEntry.id)}" alt="Locked" onerror="this.onerror=null; this.src='/assets/sprites/icon/1.png'" loading="lazy" />
                            </div>
                            <span class="pc-slot-num">${numStr}</span>
                            <span class="pc-slot-name">???</span>
                        </div>
                    `;
                }
            }).join('');

            // Click listener for unlocked slots
            slotsGrid.querySelectorAll('.pc-slot-box:not(.locked)').forEach(el => {
                el.addEventListener('click', () => {
                    const id = parseInt(el.getAttribute('data-id'), 10);
                    state.selectedPcMonId = id;
                    renderFireRedPc();
                });
            });
        }

        // Render Summary Panel
        renderPcSummaryPanel();
    }

    function renderPcSummaryPanel() {
        let mon = state.account.roster.find(m => m.id === state.selectedPcMonId);
        if (!mon) {
            mon = state.account.roster[0];
            if (mon) state.selectedPcMonId = mon.id;
        }
        if (!mon) return;

        const isStarter = (mon.id === state.account.activeStarterId);
        const starterBadge = document.getElementById('pc-starter-badge');
        if (starterBadge) {
            if (isStarter) starterBadge.classList.remove('hidden');
            else starterBadge.classList.add('hidden');
        }

        const sprite = document.getElementById('pc-mon-sprite');
        const star = document.getElementById('pc-mon-star');
        const name = document.getElementById('pc-mon-name');
        const tier = document.getElementById('pc-mon-tier');
        const nature = document.getElementById('pc-mon-nature');
        const ability = document.getElementById('pc-mon-ability');
        const shinyRow = document.getElementById('pc-shiny-toggle-wrap');
        const shinyBox = document.getElementById('pc-shiny-checkbox');
        const shinyLbl = document.getElementById('pc-shiny-status-label');
        const setStarterBtn = document.getElementById('btn-pc-set-starter');

        if (sprite) sprite.src = getSpriteUrl(mon.id, mon.isShiny);
        if (star) {
            if (mon.shinyUnlocked) star.classList.remove('hidden');
            else star.classList.add('hidden');
        }
        if (name) name.textContent = `#${mon.id} ${mon.name}`;
        if (tier) {
            tier.className = `badge-mini tier-${mon.tier}`;
            tier.textContent = mon.tier.toUpperCase();
        }
        if (nature) nature.textContent = NATURES[mon.nature] || "Hardy";
        if (ability) ability.textContent = `Slot ${mon.abilityNum + 1}`;

        // Shiny Skin Switcher
        if (shinyRow && shinyBox && shinyLbl) {
            if (mon.shinyUnlocked) {
                shinyRow.classList.remove('hidden');
                shinyBox.checked = mon.isShiny;
                shinyLbl.textContent = mon.isShiny ? 'Attiva (Colori Shiny)' : 'Disattivata (Colori Normali + Stella)';
            } else {
                shinyRow.classList.add('hidden');
            }
        }

        // IV Bars & Radar Graph Rendering
        if (!mon.ivs) mon.ivs = generateRandomIvs();
        const ivStats = ['hp', 'atk', 'def', 'spAtk', 'spDef', 'speed'];
        const ivIds = ['iv-hp', 'iv-atk', 'iv-def', 'iv-spa', 'iv-spd', 'iv-spe'];

        let ivSum = 0;
        ivStats.forEach((stat, i) => {
            const val = mon.ivs[stat] || 0;
            ivSum += val;
            const fill = document.getElementById(`${ivIds[i]}-fill`);
            const num = document.getElementById(`${ivIds[i]}-val`);
            const rankEl = document.getElementById(`${ivIds[i]}-rank`);
            const gr = getIvGrade(val);

            if (fill && num) {
                const pct = Math.round((val / 31) * 100);
                fill.style.width = `${pct}%`;
                if (val >= 31) fill.className = 'iv-fill max';
                else fill.className = 'iv-fill';
                num.textContent = val;
            }
            if (rankEl) {
                rankEl.textContent = gr.grade;
                rankEl.style.color = gr.color;
                rankEl.style.borderColor = gr.color;
                rankEl.style.backgroundColor = gr.bg;
            }
        });

        // Total IV Badge
        const totGr = getTotalIvGrade(ivSum);
        const totBadge = document.getElementById('pc-mon-iv-tot-badge');
        if (totBadge) {
            totBadge.innerHTML = `RANK: <strong style="color:${totGr.color}">${totGr.grade}</strong> (${ivSum}/186)`;
        }

        // Render In-Game Style IV Radar Graph
        const radarContainer = document.getElementById('iv-radar-container');
        if (radarContainer) {
            radarContainer.innerHTML = renderIvRadarGraphSvg(mon.ivs);
        }

        // Mega Stone Section (Surge Defeat Required)
        const megaContainer = document.getElementById('pc-mega-section');
        const dexEntry = state.pokedex.find(p => p.id === mon.id);
        const hasMega = dexEntry && dexEntry.hasMega;
        const megas = dexEntry ? dexEntry.megaStones : [];

        if (megaContainer) {
            if (!hasMega || megas.length === 0) {
                megaContainer.innerHTML = '';
            } else if (!mon.beatSurge) {
                megaContainer.innerHTML = `
                    <div class="pc-upgrade-line" style="flex-direction:column; align-items:flex-start;">
                        <span class="badge-mini" style="background:rgba(255,255,255,0.06); color:var(--text-muted);">
                            🔒 Megapietra Bloccata
                        </span>
                        <small style="color:var(--text-muted); font-size:0.75rem;">Batti Lt. Surge con questo Pokémon per sbloccare l'acquisto!</small>
                    </div>
                `;
            } else {
                const buttons = megas.map(stone => {
                    const unlocked = (mon.megaUnlocked && mon.megaUnlocked.includes(stone.id));
                    if (unlocked) {
                        return `<span class="badge-mini badge-surge-beaten">✅ ${stone.name} Sbloccata</span>`;
                    } else {
                        const tier = (mon.tier === 'legendary') ? 'legendary' : 'epic';
                        const cost = 15;
                        const canBuy = state.account.devMode || (state.account.rarityPoints[tier] >= cost);
                        return `
                            <button class="btn-xs btn-buy-mega-pc" data-mon="${mon.id}" data-stone="${stone.id}" data-tier="${tier}" ${canBuy ? '' : 'disabled'}>
                                💎 Sblocca ${stone.name} (15 Pti ${tier.toUpperCase()})
                            </button>
                        `;
                    }
                }).join('');

                megaContainer.innerHTML = `
                    <div class="pc-upgrade-line" style="flex-direction:column; align-items:flex-start; gap:4px;">
                        <span class="badge-mini badge-surge-beaten">⚡ Surge Sconfitto!</span>
                        ${buttons}
                    </div>
                `;

                megaContainer.querySelectorAll('.btn-buy-mega-pc').forEach(btn => {
                    btn.addEventListener('click', () => {
                        const monId = parseInt(btn.getAttribute('data-mon'), 10);
                        const stoneId = parseInt(btn.getAttribute('data-stone'), 10);
                        const ptsTier = btn.getAttribute('data-tier');

                        if (!state.account.devMode) {
                            if ((state.account.rarityPoints[ptsTier] || 0) < 15) {
                                alert(`Punti ${ptsTier.toUpperCase()} insufficienti (Richiesti: 15).`);
                                return;
                            }
                            state.account.rarityPoints[ptsTier] -= 15;
                        }
                        if (!mon.megaUnlocked) mon.megaUnlocked = [];
                        if (!mon.megaUnlocked.includes(stoneId)) mon.megaUnlocked.push(stoneId);

                        saveAccount();
                        renderFireRedPc();
                        renderProfileAndHub();
                        alert(`Megapietra sbloccata! Riceverai la pietra e il Bracciale Mega all'inizio della run!`);
                    });
                });
            }
        }

        if (setStarterBtn) {
            if (isStarter) {
                setStarterBtn.textContent = '⭐ Starter Attivo';
                setStarterBtn.disabled = true;
                setStarterBtn.classList.remove('btn-primary');
                setStarterBtn.classList.add('btn-secondary');
            } else {
                setStarterBtn.textContent = '⭐ Seleziona come Starter Run';
                setStarterBtn.disabled = false;
                setStarterBtn.classList.remove('btn-secondary');
                setStarterBtn.classList.add('btn-primary');
            }
        }
    }

    // =========================================================================
    // Tab 3: Gacha Machine with Authentic Egg Animations & Regional Banners
    // =========================================================================
    function initGachaMachine() {
        // Banner selection
        document.querySelectorAll('.banner-pill').forEach(pill => {
            pill.addEventListener('click', () => {
                const bId = parseInt(pill.getAttribute('data-banner'), 10);
                state.selectedGachaBanner = bId;

                document.querySelectorAll('.banner-pill').forEach(p => p.classList.remove('active'));
                pill.classList.add('active');

                const bannerInfo = BANNER_NAMES.find(b => b.id === bId) || BANNER_NAMES[0];
                document.getElementById('active-banner-title').textContent = bannerInfo.title;
                document.getElementById('active-banner-desc').textContent = bannerInfo.desc;
            });
        });

        // Pull buttons
        document.getElementById('btn-gacha-pull-1')?.addEventListener('click', () => performGachaPull(1));
        document.getElementById('btn-gacha-pull-10')?.addEventListener('click', () => performGachaPull(10));

        // Shard to ticket purchase
        document.getElementById('btn-buy-ticket-shards')?.addEventListener('click', () => {
            if (state.account.shards >= 100) {
                state.account.shards -= 100;
                state.account.tickets += 1;
                saveAccount();
                renderProfileAndHub();
                alert('Acquistato 1 Biglietto Gacha per 100 Frammenti!');
            } else {
                alert('Frammenti Rogue insufficienti (richiesti 100 💎).');
            }
        });

        // Info Tassi Modal toggle & close
        const ratesModal = document.getElementById('gacha-rates-modal');
        document.getElementById('btn-toggle-gacha-rates')?.addEventListener('click', () => {
            if (!ratesModal) return;
            ratesModal.classList.toggle('hidden');
        });
        document.getElementById('btn-close-gacha-rates')?.addEventListener('click', () => {
            if (!ratesModal) return;
            ratesModal.classList.add('hidden');
        });
    }

    // Restore idle pod view
    function restoreGachaIdleStage() {
        const landingArea = document.getElementById('egg-landing-area');
        if (!landingArea) return;
        landingArea.innerHTML = `
            <div class="idle-gacha-pod">
                <div class="idle-egg"><img src="/assets/egg_common.png" alt="Egg" class="gacha-egg-img" /></div>
                <span class="idle-gacha-title">Pronto all'estrazione!</span>
                <span class="idle-gacha-sub">Seleziona 1x o 10x per iniziare l'estrazione regionale.</span>
            </div>
        `;
        // Re-enable pull buttons
        const btn1 = document.getElementById('btn-gacha-pull-1');
        const btn10 = document.getElementById('btn-gacha-pull-10');
        if (btn1) btn1.disabled = false;
        if (btn10) btn10.disabled = false;
    }

    async function performGachaPull(count) {
        if (!state.account.devMode) {
            if (state.account.tickets < count) {
                alert(`Non hai abbastanza biglietti (${state.account.tickets}/${count}). Puoi scambiare 100 Frammenti 💎 per un biglietto.`);
                return;
            }
            state.account.tickets -= count;
            saveAccount();
            updateWalletUI();
        }

        // Disable pull buttons during sequence
        const btn1 = document.getElementById('btn-gacha-pull-1');
        const btn10 = document.getElementById('btn-gacha-pull-10');
        if (btn1) btn1.disabled = true;
        if (btn10) btn10.disabled = true;

        // Step 1: Filter pool by selected regional banner
        let bannerPool = state.pokedex;
        if (state.selectedGachaBanner >= 1 && state.selectedGachaBanner <= 9) {
            bannerPool = state.pokedex.filter(p => p.gen === state.selectedGachaBanner);
        }
        if (bannerPool.length === 0) bannerPool = state.pokedex;

        // Step 2: Roll pulls with random IVs and duplicate rules
        const results = [];
        for (let i = 0; i < count; i++) {
            // Tier rates: Common 60%, Uncommon 25%, Rare 10%, Epic 4%, Legendary 1%
            const roll = Math.random() * 100;
            let rolledTier = 'common';
            if (roll < 1) rolledTier = 'legendary';
            else if (roll < 5) rolledTier = 'epic';
            else if (roll < 15) rolledTier = 'rare';
            else if (roll < 40) rolledTier = 'uncommon';

            let pool = bannerPool.filter(p => p.tier === rolledTier);
            if (pool.length === 0) pool = bannerPool;
            const chosen = pool[Math.floor(Math.random() * pool.length)] || { id: 1, name: "Bulbasaur", tier: "common" };

            // Shiny roll: 1 in 128
            const isShiny = (Math.floor(Math.random() * 128) === 0);

            // Roll random IVs for this pull (0 to 31)
            const newIvs = generateRandomIvs();

            // Duplicate handling
            const existing = state.account.roster.find(m => m.id === chosen.id);
            let isDuplicate = false;
            let pointsAwarded = 0;
            const ivUpgrades = [];

            if (existing) {
                isDuplicate = true;
                pointsAwarded = DUP_POINTS[rolledTier] || 5;
                state.account.rarityPoints[rolledTier] = (state.account.rarityPoints[rolledTier] || 0) + pointsAwarded;

                // Merge IVs: keep highest between existing and new!
                if (!existing.ivs) existing.ivs = generateRandomIvs();
                ['hp', 'atk', 'def', 'spAtk', 'spDef', 'speed'].forEach(stat => {
                    if (newIvs[stat] > existing.ivs[stat]) {
                        ivUpgrades.push({ stat, old: existing.ivs[stat], next: newIvs[stat] });
                        existing.ivs[stat] = newIvs[stat];
                    }
                });

                // Unlock shiny if duplicate is shiny
                if (isShiny && !existing.shinyUnlocked) {
                    existing.shinyUnlocked = true;
                    existing.isShiny = true;
                }
            } else {
                // New unlock
                state.account.roster.push({
                    id: chosen.id,
                    key: chosen.key || chosen.name.toUpperCase(),
                    name: chosen.name,
                    tier: chosen.tier,
                    nature: Math.floor(Math.random() * 25),
                    abilityNum: 0,
                    isShiny: isShiny,
                    shinyUnlocked: isShiny,
                    beatSurge: false,
                    megaUnlocked: [],
                    ivs: newIvs,
                    happiness: 255
                });
            }

            results.push({
                mon: chosen,
                tier: rolledTier,
                isShiny: isShiny,
                isDuplicate: isDuplicate,
                pointsAwarded: pointsAwarded,
                ivs: newIvs,
                ivUpgrades: ivUpgrades
            });
        }

        saveAccount();
        renderProfileAndHub();
        renderFireRedPc();

        // Step 3: Run the Full Gacha Animation Sequence:
        // Drop Eggs -> Show 10 Opened Eggs -> Parade -> Final Recap
        await playGachaSequence(count, results);
    }

    // Master Gacha Animation Flow (Authentic GBA Sprite Frames from pokeemerald)
    async function playGachaSequence(count, results) {
        const landingArea = document.getElementById('egg-landing-area');
        if (!landingArea) return;

        // PHASE 1: Eggs Drop, Wobble & Crack using authentic frames (0 -> crack1 -> crack2)
        if (count === 1) {
            const pull = results[0];
            const intactEgg = getEggSpriteUrl(pull.tier, pull.isShiny, 'intact');
            landingArea.innerHTML = `
                <div class="animated-egg egg-dropping-single">
                    <img id="gacha-single-egg-img" src="${intactEgg}" class="animated-egg-img" alt="Egg" />
                    ${pull.isShiny ? '<span class="egg-shiny-star">★</span>' : ''}
                </div>
            `;
            await new Promise(r => setTimeout(r, 1600));

            // Crack frame 1
            const eggImgElem = document.getElementById('gacha-single-egg-img');
            if (eggImgElem) eggImgElem.src = getEggSpriteUrl(pull.tier, pull.isShiny, 'crack1');
            await new Promise(r => setTimeout(r, 400));

            // Crack frame 2
            if (eggImgElem) eggImgElem.src = getEggSpriteUrl(pull.tier, pull.isShiny, 'crack2');
            await new Promise(r => setTimeout(r, 400));
        } else {
            const eggsHtml = results.map((pull, idx) => {
                const eggImg = getEggSpriteUrl(pull.tier, pull.isShiny, 'intact');
                return `
                    <div class="animated-egg egg-dropping-single" style="animation-delay: ${idx * 0.08}s;">
                        <img id="multi-egg-img-${idx}" src="${eggImg}" class="animated-egg-img" alt="Egg" />
                        ${pull.isShiny ? '<span class="egg-shiny-star">★</span>' : ''}
                    </div>
                `;
            }).join('');

            landingArea.innerHTML = `
                <div class="multi-eggs-rack">
                    ${eggsHtml}
                </div>
            `;
            await new Promise(r => setTimeout(r, 1600));

            // Crack frame 1
            results.forEach((pull, idx) => {
                const el = document.getElementById(`multi-egg-img-${idx}`);
                if (el) el.src = getEggSpriteUrl(pull.tier, pull.isShiny, 'crack1');
            });
            await new Promise(r => setTimeout(r, 400));

            // Crack frame 2
            results.forEach((pull, idx) => {
                const el = document.getElementById(`multi-egg-img-${idx}`);
                if (el) el.src = getEggSpriteUrl(pull.tier, pull.isShiny, 'crack2');
            });
            await new Promise(r => setTimeout(r, 400));
        }

        // PHASE 2: Show 10 Opened Eggs Rack ("Le 10 uova aperte")
        const skipped = await showOpenedEggsRack(count, results);
        if (skipped) {
            showGachaRecap(results);
            return;
        }

        // PHASE 3: Pokémon Parade ("Sfilata di Pokémon uno alla volta")
        await playPokemonParade(results);

        // PHASE 4: Final Summary Recap Grid
        showGachaRecap(results);
    }

    // Phase 2: Displays the authentic opened / hatched eggs in the rack (No fake glow, pure pixel art)
    function showOpenedEggsRack(count, results) {
        return new Promise((resolve) => {
            const landingArea = document.getElementById('egg-landing-area');
            if (!landingArea) { resolve(false); return; }

            const eggsHtml = results.map((pull) => {
                const hatchedEggImg = getEggSpriteUrl(pull.tier, pull.isShiny, 'hatched');
                return `
                    <div class="opened-egg-slot tier-${pull.tier}">
                        <div class="opened-egg-visual">
                            <img src="${hatchedEggImg}" class="opened-egg-shell-img" alt="Hatched Egg" />
                            ${pull.isShiny ? '<span class="egg-shiny-star">★</span>' : ''}
                        </div>
                        <span class="opened-egg-badge ${pull.isShiny ? 'shiny' : `tier-${pull.tier}`}">
                            ${pull.isShiny ? '★ SHINY' : pull.tier.toUpperCase()}
                        </span>
                    </div>
                `;
            }).join('');

            const title = (count === 1) ? '✨ UOVO SCHIUSO! ✨' : '✨ 10 UOVA SCHIUSE! ✨';
            const sub = (count === 1)
                ? 'L\'uovo si è aperto! Guarda quale Pokémon è apparso...'
                : 'Le 10 uova si sono aperte! Inizia la sfilata o salta direttamente al riepilogo.';

            landingArea.innerHTML = `
                <div class="opened-eggs-stage">
                    <div class="opened-eggs-header">
                        <h3 class="opened-eggs-title">${title}</h3>
                        <p class="opened-eggs-sub">${sub}</p>
                    </div>
                    <div class="opened-eggs-grid">
                        ${eggsHtml}
                    </div>
                    <div class="opened-eggs-actions">
                        <button id="btn-start-parade" class="btn btn-primary btn-large">
                            Inizia Sfilata Pokémon ▶
                        </button>
                        <button id="btn-skip-parade-early" class="btn btn-secondary btn-large">
                            ⏭️ Salta Sfilata
                        </button>
                    </div>
                </div>
            `;

            let autoTimer = null;
            let resolved = false;

            const handleStart = () => {
                if (resolved) return;
                resolved = true;
                if (autoTimer) clearTimeout(autoTimer);
                resolve(false); // Do not skip parade
            };

            const handleSkip = () => {
                if (resolved) return;
                resolved = true;
                if (autoTimer) clearTimeout(autoTimer);
                resolve(true); // Skip parade directly
            };

            document.getElementById('btn-start-parade')?.addEventListener('click', handleStart);
            document.getElementById('btn-skip-parade-early')?.addEventListener('click', handleSkip);

            // Auto-advance to parade after 1.8 seconds if the user does not click
            autoTimer = setTimeout(handleStart, 1800);
        });
    }

    // Phase 3: Interactive Pokémon Parade (Showcase One-by-One)
    function playPokemonParade(results) {
        return new Promise((resolve) => {
            const landingArea = document.getElementById('egg-landing-area');
            if (!landingArea) { resolve(); return; }

            let currentIndex = 0;
            const total = results.length;

            function renderParadeCard() {
                const res = results[currentIndex];
                const ivs = res.ivs || { hp: 0, atk: 0, def: 0, spAtk: 0, spDef: 0, speed: 0 };
                const ivSum = ivs.hp + ivs.atk + ivs.def + ivs.spAtk + ivs.spDef + ivs.speed;

                const gHp  = getIvGrade(ivs.hp);
                const gAtk = getIvGrade(ivs.atk);
                const gDef = getIvGrade(ivs.def);
                const gSpa = getIvGrade(ivs.spAtk);
                const gSpd = getIvGrade(ivs.spDef);
                const gSpe = getIvGrade(ivs.speed);
                const gTot = getTotalIvGrade(ivSum);

                // Progress pips
                const pipsHtml = results.map((_, i) => {
                    let cls = 'parade-pip';
                    if (i < currentIndex) cls += ' seen';
                    else if (i === currentIndex) cls += ' active';
                    return `<span class="${cls}"></span>`;
                }).join('');

                // Duplicate / New badge
                const statusHtml = res.isDuplicate
                    ? `<div class="parade-status-tag dup">🔄 DOPPIONE (+${res.pointsAwarded} PUNTI RARITÀ)</div>`
                    : `<div class="parade-status-tag new">✨ NUOVO POKÉMON AGGIUNTO AL PC! ✨</div>`;

                // Upgraded IVs note if duplicate merged higher stats
                let upgradeHtml = '';
                if (res.isDuplicate && res.ivUpgrades && res.ivUpgrades.length > 0) {
                    const ups = res.ivUpgrades.map(u => {
                        const oG = getIvGrade(u.old);
                        const nG = getIvGrade(u.next);
                        return `${u.stat.toUpperCase()}: <strong style="color:${oG.color}">${oG.grade}</strong>➔<strong style="color:${nG.color}">${nG.grade}</strong>`;
                    }).join(' | ');
                    upgradeHtml = `<div class="parade-upgrades-note">⬆️ Statistiche IV Migliorate: ${ups}</div>`;
                }

                const isLast = (currentIndex === total - 1);
                const nextBtnText = isLast
                    ? '🏆 Mostra Riepilogo Completo'
                    : `Prossimo Pokémon ▶ (${currentIndex + 2}/${total})`;

                landingArea.innerHTML = `
                    <div class="parade-stage">
                        <div class="parade-progress-bar">
                            <span class="parade-counter-tag">POKÉMON ${currentIndex + 1} DI ${total}</span>
                            <div class="parade-pips">${pipsHtml}</div>
                        </div>

                        <div class="parade-mon-card tier-${res.tier}" id="parade-mon-card">
                            <div class="parade-sprite-stage">
                                <img src="${getSpriteUrl(res.mon.id, res.isShiny)}" alt="${res.mon.name}" class="parade-sprite-img" />
                                ${res.isShiny ? '<span class="parade-shiny-tag">★ SHINY</span>' : ''}
                            </div>

                            <div class="parade-mon-headline">
                                <span class="parade-mon-name">#${res.mon.id} ${res.mon.name.toUpperCase()}</span>
                                <span class="badge-mini tier-${res.tier}">${res.tier.toUpperCase()}</span>
                            </div>

                            ${statusHtml}
                            ${upgradeHtml}

                            <div class="parade-ivs-row">
                                <span class="parade-iv-chip" style="color:${gHp.color}; border-color:${gHp.color}; background:${gHp.bg}">HP <strong>${gHp.grade}</strong></span>
                                <span class="parade-iv-chip" style="color:${gAtk.color}; border-color:${gAtk.color}; background:${gAtk.bg}">ATK <strong>${gAtk.grade}</strong></span>
                                <span class="parade-iv-chip" style="color:${gDef.color}; border-color:${gDef.color}; background:${gDef.bg}">DEF <strong>${gDef.grade}</strong></span>
                                <span class="parade-iv-chip" style="color:${gSpa.color}; border-color:${gSpa.color}; background:${gSpa.bg}">SPA <strong>${gSpa.grade}</strong></span>
                                <span class="parade-iv-chip" style="color:${gSpd.color}; border-color:${gSpd.color}; background:${gSpd.bg}">SPD <strong>${gSpd.grade}</strong></span>
                                <span class="parade-iv-chip" style="color:${gSpe.color}; border-color:${gSpe.color}; background:${gSpe.bg}">SPE <strong>${gSpe.grade}</strong></span>
                                <span class="parade-iv-chip total" style="color:${gTot.color}; border-color:${gTot.color}; background:${gTot.bg}">TOT <strong>${gTot.grade}</strong></span>
                            </div>
                        </div>

                        <div class="parade-controls-bar">
                            <button id="btn-parade-next" class="btn btn-primary btn-large">
                                ${nextBtnText}
                            </button>
                            <button id="btn-parade-skip" class="btn btn-secondary btn-large">
                                ⏭️ Salta Sfilata
                            </button>
                        </div>
                        <span class="parade-kbd-hint">💡 Clicca sulla scheda o usa [Spazio / Invio] per avanzare, [Esc] per saltare</span>
                    </div>
                `;

                // Bind events
                document.getElementById('btn-parade-next')?.addEventListener('click', (e) => {
                    e.stopPropagation();
                    advance();
                });
                document.getElementById('btn-parade-skip')?.addEventListener('click', (e) => {
                    e.stopPropagation();
                    cleanup();
                    resolve();
                });
                document.getElementById('parade-mon-card')?.addEventListener('click', () => {
                    advance();
                });
            }

            function advance() {
                if (currentIndex < total - 1) {
                    currentIndex++;
                    renderParadeCard();
                } else {
                    cleanup();
                    resolve();
                }
            }

            function onKeyDown(e) {
                if (e.code === 'Space' || e.code === 'Enter' || e.code === 'ArrowRight') {
                    e.preventDefault();
                    advance();
                } else if (e.code === 'Escape') {
                    e.preventDefault();
                    cleanup();
                    resolve();
                }
            }

            function cleanup() {
                window.removeEventListener('keydown', onKeyDown);
            }

            window.addEventListener('keydown', onKeyDown);
            renderParadeCard();
        });
    }

    // Phase 4: Final Summary Recap Grid (Unified Screen)
    function showGachaRecap(results) {
        const landingArea = document.getElementById('egg-landing-area');
        if (!landingArea) return;

        const cardsHtml = results.map(res => {
            const ivs = res.ivs || { hp: 0, atk: 0, def: 0, spAtk: 0, spDef: 0, speed: 0 };
            const ivSum = ivs.hp + ivs.atk + ivs.def + ivs.spAtk + ivs.spDef + ivs.speed;

            const gHp  = getIvGrade(ivs.hp);
            const gAtk = getIvGrade(ivs.atk);
            const gDef = getIvGrade(ivs.def);
            const gSpa = getIvGrade(ivs.spAtk);
            const gSpd = getIvGrade(ivs.spDef);
            const gSpe = getIvGrade(ivs.speed);
            const gTot = getTotalIvGrade(ivSum);

            const badge = res.isDuplicate
                ? `<span class="pull-badge-sm dup">+${res.pointsAwarded} Pti</span>`
                : `<span class="pull-badge-sm new">NUOVO</span>`;

            return `
                <div class="recap-card tier-${res.tier}">
                    <div class="recap-sprite-wrap">
                        <img src="${getSpriteUrl(res.mon.id, res.isShiny)}" alt="${res.mon.name}" class="recap-sprite-img" />
                        ${res.isShiny ? '<span class="egg-shiny-sparkle">★</span>' : ''}
                    </div>
                    <span class="recap-name">#${res.mon.id} ${res.mon.name}</span>
                    <div class="recap-badges-line">
                        <span class="badge-mini tier-${res.tier}">${res.tier.toUpperCase()}</span>
                        ${badge}
                    </div>
                    <div class="recap-ivs-line">
                        <span class="recap-iv-pip" style="color:${gHp.color}; border-color:${gHp.color}">H:<strong>${gHp.grade}</strong></span>
                        <span class="recap-iv-pip" style="color:${gAtk.color}; border-color:${gAtk.color}">A:<strong>${gAtk.grade}</strong></span>
                        <span class="recap-iv-pip" style="color:${gDef.color}; border-color:${gDef.color}">D:<strong>${gDef.grade}</strong></span>
                        <span class="recap-iv-pip" style="color:${gSpa.color}; border-color:${gSpa.color}">C:<strong>${gSpa.grade}</strong></span>
                        <span class="recap-iv-pip" style="color:${gSpd.color}; border-color:${gSpd.color}">S:<strong>${gSpd.grade}</strong></span>
                        <span class="recap-iv-pip" style="color:${gSpe.color}; border-color:${gSpe.color}">V:<strong>${gSpe.grade}</strong></span>
                        <span class="recap-iv-pip" style="color:${gTot.color}; border-color:${gTot.color}; font-weight:bold;">T:<strong>${gTot.grade}</strong></span>
                    </div>
                </div>
            `;
        }).join('');

        landingArea.innerHTML = `
            <div class="gacha-summary-recap">
                <div class="recap-header">
                    <h3>🏆 RIEPILOGO ESTRAZIONE (${results.length} POKÉMON)</h3>
                    <p>Tutti i Pokémon estratti sono stati registrati nel tuo PC Box e i doppioni convertiti in Punti Rarità.</p>
                </div>
                <div class="recap-grid-10">
                    ${cardsHtml}
                </div>
                <div class="recap-actions-bar">
                    <button id="btn-recap-pull-1" class="btn btn-primary">
                        🥚 Tira Ancora 1x
                    </button>
                    <button id="btn-recap-pull-10" class="btn btn-secondary">
                        🥚🥚 Tira Ancora 10x
                    </button>
                    <button id="btn-recap-go-pc" class="btn btn-secondary nav-trigger" data-tab="roster">
                        📦 Vai al PC Box
                    </button>
                    <button id="btn-recap-close" class="btn btn-secondary">
                        ↩️ Schermata Principale
                    </button>
                </div>
            </div>
        `;

        // Re-enable bottom action buttons
        const btn1 = document.getElementById('btn-gacha-pull-1');
        const btn10 = document.getElementById('btn-gacha-pull-10');
        if (btn1) btn1.disabled = false;
        if (btn10) btn10.disabled = false;

        // Recap buttons
        document.getElementById('btn-recap-pull-1')?.addEventListener('click', () => performGachaPull(1));
        document.getElementById('btn-recap-pull-10')?.addEventListener('click', () => performGachaPull(10));
        document.getElementById('btn-recap-close')?.addEventListener('click', () => restoreGachaIdleStage());
        document.getElementById('btn-recap-go-pc')?.addEventListener('click', () => {
            const pcTabBtn = document.querySelector('.nav-tab[data-tab="roster"]');
            if (pcTabBtn) pcTabBtn.click();
        });
    }

    // =========================================================================
    // Tab 4: Market & Bag Setup
    // =========================================================================
    function initMarketShop() {}

    function renderMarket() {
        const grid = document.getElementById('market-items-grid');
        const bagList = document.getElementById('bag-equipped-list');
        if (!grid || !bagList) return;

        grid.innerHTML = SHOP_ITEMS.map(item => {
            const canAfford = state.account.devMode || (state.account.money >= item.price);
            return `
                <div class="market-item-card">
                    <div>
                        <div class="market-item-title">${item.icon} ${item.name}</div>
                        <div class="market-item-desc">${item.desc}</div>
                    </div>
                    <div class="market-item-foot">
                        <span class="item-price">₽ ${item.price.toLocaleString()}</span>
                        <button class="btn btn-secondary btn-sm btn-buy-shop-item" data-id="${item.id}" data-price="${item.price}" ${canAfford ? '' : 'disabled'}>
                            Acquista
                        </button>
                    </div>
                </div>
            `;
        }).join('');

        const equipped = state.account.equippedBagItems || [];
        const ownedKeys = Object.keys(state.account.purchasedItems || {}).map(Number).filter(id => state.account.purchasedItems[id] > 0);

        if (ownedKeys.length === 0) {
            bagList.innerHTML = `<div style="color:var(--text-muted); font-size:0.85rem;">Nessuno strumento acquistato. Comprane uno nel negozio per equipaggiarlo!</div>`;
        } else {
            bagList.innerHTML = ownedKeys.map(itemId => {
                const def = SHOP_ITEMS.find(i => i.id === itemId) || { name: `Strumento #${itemId}`, icon: "📦" };
                const isEquipped = equipped.includes(itemId);

                return `
                    <div class="bag-slot-item">
                        <span>${def.icon} <strong>${def.name}</strong></span>
                        <button class="btn-xs btn-toggle-equip" data-id="${itemId}">
                            ${isEquipped ? '✅ Equipaggiato' : '➕ Equipaggia'}
                        </button>
                    </div>
                `;
            }).join('');
        }

        document.querySelectorAll('.btn-buy-shop-item').forEach(btn => {
            btn.addEventListener('click', () => {
                const id = parseInt(btn.getAttribute('data-id'), 10);
                const price = parseInt(btn.getAttribute('data-price'), 10);
                if (state.account.devMode || state.account.money >= price) {
                    if (!state.account.devMode) {
                        state.account.money -= price;
                    }
                    state.account.purchasedItems[id] = (state.account.purchasedItems[id] || 0) + 1;
                    if (!state.account.equippedBagItems.includes(id) && state.account.equippedBagItems.length < 4) {
                        state.account.equippedBagItems.push(id);
                    }
                    saveAccount();
                    renderMarket();
                    renderPrerunSummary();
                }
            });
        });

        document.querySelectorAll('.btn-toggle-equip').forEach(btn => {
            btn.addEventListener('click', () => {
                const id = parseInt(btn.getAttribute('data-id'), 10);
                const idx = state.account.equippedBagItems.indexOf(id);
                if (idx >= 0) {
                    state.account.equippedBagItems.splice(idx, 1);
                } else {
                    if (state.account.equippedBagItems.length >= 4) {
                        alert('Puoi equipaggiare al massimo 4 strumenti nella borsa di partenza.');
                        return;
                    }
                    state.account.equippedBagItems.push(id);
                }
                saveAccount();
                renderMarket();
                renderPrerunSummary();
            });
        });
    }

    // =========================================================================
    // Tab 5: Prerun Summary, Injection & Emulator Launcher
    // =========================================================================
    function renderPrerunSummary() {
        const starter = state.account.roster.find(m => m.id === state.account.activeStarterId) || state.account.roster[0];
        if (!starter) return;

        const sprite = document.getElementById('prerun-starter-sprite');
        const name = document.getElementById('prerun-starter-name');
        const shiny = document.getElementById('prerun-starter-shiny');
        const stats = document.getElementById('prerun-starter-stats');
        const mega = document.getElementById('prerun-starter-mega');
        const bagUl = document.getElementById('prerun-bag-list');

        if (sprite) sprite.src = getSpriteUrl(starter.id, starter.isShiny);
        if (name) name.textContent = starter.name;
        if (shiny) {
            if (starter.shinyUnlocked) shiny.classList.remove('hidden');
            else shiny.classList.add('hidden');
        }
        if (stats) {
            const nat = NATURES[starter.nature] || "Hardy";
            stats.innerHTML = `Natura: <strong>${nat}</strong> | Abilità: <strong>Slot ${starter.abilityNum + 1}</strong> | Felicità: <strong>255</strong>`;
        }
        if (mega) {
            if (starter.megaUnlocked && starter.megaUnlocked.length > 0) {
                mega.className = 'badge-mini badge-surge-beaten';
                mega.textContent = 'Megapietra & Bracciale Pronti 💎';
            } else {
                mega.textContent = '';
            }
        }
        if (bagUl) {
            const equipped = state.account.equippedBagItems || [];
            if (equipped.length === 0) {
                bagUl.innerHTML = `<li>Nessuno strumento di setup equipaggiato</li>`;
            } else {
                bagUl.innerHTML = equipped.map(id => {
                    const it = SHOP_ITEMS.find(s => s.id === id);
                    return `<li>${it ? it.icon : '📦'} <strong>${it ? it.name : `Strumento #${id}`}</strong></li>`;
                }).join('');
            }
        }
    }

    function initDropzone() {
        const dropzone = document.getElementById('dropzone');
        const fileInput = document.getElementById('rom-file-input');
        if (!dropzone || !fileInput) return;

        ['dragenter', 'dragover'].forEach(name => {
            dropzone.addEventListener(name, (e) => { e.preventDefault(); dropzone.classList.add('dragover'); });
        });
        ['dragleave', 'drop'].forEach(name => {
            dropzone.addEventListener(name, (e) => { e.preventDefault(); dropzone.classList.remove('dragover'); });
        });

        dropzone.addEventListener('drop', (e) => {
            const files = e.dataTransfer.files;
            if (files && files.length > 0) handleRomUpload(files[0]);
        });

        fileInput.addEventListener('change', (e) => {
            if (e.target.files && e.target.files.length > 0) handleRomUpload(e.target.files[0]);
        });
    }

    async function handleRomUpload(file) {
        const infoBox = document.getElementById('rom-info-box');
        const badge = document.getElementById('rom-badge');
        const name = document.getElementById('rom-name');
        const msg = document.getElementById('rom-status-msg');

        if (infoBox) infoBox.classList.remove('hidden', 'verified', 'warning', 'invalid');
        if (name) name.textContent = file.name;

        try {
            const arrayBuf = await file.arrayBuffer();
            const bytes = new Uint8Array(arrayBuf);
            state.baseRomBuffer = bytes;

            let title = '';
            for (let i = 0xA0; i < 0xAC; i++) {
                if (bytes[i] === 0) break;
                title += String.fromCharCode(bytes[i]);
            }
            let code = '';
            for (let i = 0xAC; i < 0xB0; i++) code += String.fromCharCode(bytes[i]);
            const crc = calculateCrc32(bytes);

            document.getElementById('detail-title').textContent = title || '(Vuoto)';
            document.getElementById('detail-code').textContent = code || '(Vuoto)';
            document.getElementById('detail-size').textContent = `${(file.size / (1024 * 1024)).toFixed(1)} MB`;
            document.getElementById('detail-crc').textContent = crc.toUpperCase();

            if (code.startsWith('BPR') || title.includes('FIRE') || title.includes('ROSSO') || crc === 'a3d2e261') {
                infoBox.classList.add('verified');
                badge.className = 'rom-info-badge verified';
                badge.textContent = 'ROM VALIDA VERIFICATA';
                msg.innerHTML = `<strong>Compatibilità confermata!</strong> Preparazione pacchetto KaizoRogue in memoria...`;
                await prepareKaizoRom();
            } else {
                infoBox.classList.add('invalid');
                badge.className = 'rom-info-badge invalid';
                badge.textContent = 'ROM NON COMPATIBILE';
                msg.textContent = `La ROM fornita non sembra essere Pokémon FireRed o Rosso Fuoco.`;
            }
        } catch (err) {
            console.error('Errore lettura ROM:', err);
        }
    }

    async function prepareKaizoRom() {
        const progressBox = document.getElementById('patch-action-box');
        const fill = document.getElementById('progress-fill');
        const txt = document.getElementById('progress-text');
        const pct = document.getElementById('progress-percent');

        if (progressBox) progressBox.classList.remove('hidden');

        try {
            const res = await fetch('patch/kaizo.gba.gz');
            if (!res.ok) throw new Error(`HTTP ${res.status}`);

            const ds = new DecompressionStream('gzip');
            const decompStream = res.body.pipeThrough(ds);
            const reader = decompStream.getReader();
            const chunks = [];
            let total = 0;

            while (true) {
                const { done, value } = await reader.read();
                if (done) break;
                chunks.push(value);
                total += value.length;
                if (fill && pct) {
                    const p = Math.min(95, Math.round((total / (32 * 1024 * 1024)) * 100));
                    fill.style.width = `${p}%`;
                    pct.textContent = `${p}%`;
                }
            }

            state.kaizoRomBuffer = new Uint8Array(total);
            let off = 0;
            for (const ch of chunks) {
                state.kaizoRomBuffer.set(ch, off);
                off += ch.length;
            }

            if (fill) fill.style.width = '100%';
            if (pct) pct.textContent = '100%';
            if (txt) txt.textContent = 'Motore KaizoRogue pronto in memoria!';

            state.patchReady = true;

            setTimeout(() => {
                document.getElementById('setup-panel')?.classList.add('hidden');
                document.getElementById('prerun-ready-panel')?.classList.remove('hidden');
                renderPrerunSummary();
            }, 400);

        } catch (err) {
            console.error('Errore decompression:', err);
            if (txt) txt.textContent = 'Errore di download pacchetto: ' + err.message;
        }
    }

    // Prerun Injection into Memory before launching game
    function injectPrerunConfigIntoRomBuffer() {
        if (!state.kaizoRomBuffer) return false;

        const starter = state.account.roster.find(m => m.id === state.account.activeStarterId) || state.account.roster[0];
        if (!starter) return false;

        const offset = (state.meta && state.meta.prerunOffset) ? state.meta.prerunOffset : 7626328;
        const dv = new DataView(state.kaizoRomBuffer.buffer, state.kaizoRomBuffer.byteOffset, state.kaizoRomBuffer.byteLength);

        let flags = 1; // active
        if (starter.shinyUnlocked) flags |= (1 << 1);
        if (starter.shinyUnlocked && !starter.isShiny) flags |= (1 << 2); // shiny skin off

        const hasMega = starter.megaUnlocked && starter.megaUnlocked.length > 0;
        if (hasMega) flags |= (1 << 3);

        let heldItem = 0;
        if (hasMega) {
            heldItem = starter.megaUnlocked[0];
        } else if (state.account.equippedBagItems.length > 0) {
            heldItem = state.account.equippedBagItems[0];
        }

        if (!starter.ivs) starter.ivs = generateRandomIvs();

        // Write KaizoWebPrerunConfig into ROM buffer
        dv.setUint32(offset + 0x00, 0x4B5A5247, true); // magic
        dv.setUint16(offset + 0x04, 1, true);          // version
        dv.setUint16(offset + 0x06, flags, true);      // flags
        dv.setUint16(offset + 0x08, starter.id, true); // starterSpecies
        dv.setUint8(offset + 0x0A, 5);                 // starterLevel (5)
        dv.setUint8(offset + 0x0B, starter.nature);    // starterNature
        dv.setUint8(offset + 0x0C, starter.abilityNum);// starterAbilityNum
        dv.setUint8(offset + 0x0D, 255);               // starterHappiness (255 MAX)
        dv.setUint8(offset + 0x0E, starter.ivs.hp);    // ivHp (actual rolled IV)
        dv.setUint8(offset + 0x0F, starter.ivs.atk);   // ivAtk
        dv.setUint8(offset + 0x10, starter.ivs.def);   // ivDef
        dv.setUint8(offset + 0x11, starter.ivs.speed); // ivSpeed
        dv.setUint8(offset + 0x12, starter.ivs.spAtk); // ivSpAtk
        dv.setUint8(offset + 0x13, starter.ivs.spDef); // ivSpDef
        dv.setUint16(offset + 0x14, heldItem, true);   // starterHeldItem
        dv.setUint16(offset + 0x16, 0, true);          // custom move 1
        dv.setUint16(offset + 0x18, 0, true);          // custom move 2

        const bag = state.account.equippedBagItems || [];
        for (let i = 0; i < 16; i++) {
            const it = (i < bag.length) ? bag[i] : 0;
            const qty = (it > 0) ? 1 : 0;
            dv.setUint16(offset + 0x1A + (i * 2), it, true);
            dv.setUint16(offset + 0x3A + (i * 2), qty, true);
        }

        const runSeed = Math.floor(Math.random() * 0xFFFFFFFF);
        dv.setUint32(offset + 0x5C, runSeed, true);

        console.log(`[KaizoRogue] Iniezione Prerun completata: ${starter.name} (#${starter.id}), IVs=[${starter.ivs.hp},${starter.ivs.atk},${starter.ivs.def},${starter.ivs.spAtk},${starter.ivs.spDef},${starter.ivs.speed}], Flags=${flags}, Seed=${runSeed.toString(16)}`);
        return true;
    }

    function initPlayRunner() {
        document.getElementById('btn-launch-game')?.addEventListener('click', () => {
            if (!state.patchReady) {
                alert('Carica prima la ROM base di FireRed / Rosso Fuoco per preparare il pacchetto.');
                return;
            }
            injectPrerunConfigIntoRomBuffer();
            launchEmulator();
        });

        document.getElementById('btn-download-rom')?.addEventListener('click', () => {
            if (!state.kaizoRomBuffer) return;
            injectPrerunConfigIntoRomBuffer();
            const blob = new Blob([state.kaizoRomBuffer], { type: 'application/octet-stream' });
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = 'Pokemon_KaizoRogue_CustomRun.gba';
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
            URL.revokeObjectURL(url);
        });

        document.getElementById('btn-fullscreen')?.addEventListener('click', () => {
            const frame = document.getElementById('game-frame');
            if (frame) {
                if (!document.fullscreenElement) frame.requestFullscreen();
                else document.exitFullscreen();
            }
        });

        document.getElementById('btn-stop-game')?.addEventListener('click', () => {
            if (confirm('Vuoi terminare la run attuale e tornare all\'Hub?')) {
                syncPostrunResults();
                window.location.reload();
            }
        });

        document.getElementById('btn-sync-postrun')?.addEventListener('click', () => {
            syncPostrunResults();
        });
    }

    function launchEmulator() {
        document.getElementById('prerun-ready-panel')?.classList.add('hidden');
        const emuPanel = document.getElementById('emulator-panel');
        if (emuPanel) {
            emuPanel.classList.remove('hidden');
            emuPanel.scrollIntoView({ behavior: 'smooth' });
        }

        state.kaizoRomBlob = new Blob([state.kaizoRomBuffer], { type: 'application/octet-stream' });
        if (state.kaizoRomUrl) URL.revokeObjectURL(state.kaizoRomUrl);
        state.kaizoRomUrl = URL.createObjectURL(state.kaizoRomBlob);

        window.EJS_player = '#game';
        window.EJS_core = 'gba';
        window.EJS_gameName = 'Pokemon KaizoRogue';
        window.EJS_color = '#00e5ff';
        window.EJS_startOnLoaded = true;
        window.EJS_pathtodata = 'https://cdn.emulatorjs.org/stable/data/';
        window.EJS_gameUrl = state.kaizoRomUrl;
        window.EJS_volume = 0.8;
        window.EJS_language = 'en-US';

        if (!document.getElementById('ejs-loader')) {
            const script = document.createElement('script');
            script.id = 'ejs-loader';
            script.src = 'https://cdn.emulatorjs.org/stable/data/loader.js';
            script.async = true;
            document.body.appendChild(script);
            console.log('[KaizoRogue] Caricamento mGBA in corso...');
        }
    }

    async function syncPostrunResults() {
        const starter = state.account.roster.find(m => m.id === state.account.activeStarterId);

        state.account.runsCount += 1;
        state.account.shards += 80;
        state.account.tickets += 1;
        state.account.money += 1500;

        if (starter) {
            starter.beatSurge = true;
            state.account.surgeWinsCount += 1;
        }

        saveAccount();
        renderProfileAndHub();
        renderFireRedPc();
        alert(`📊 Risultati Run Sincronizzati!\n+80 Frammenti 💎\n+1 Biglietto Gacha 🎟️\n+$1,500 💰\nLt. Surge Sconfitto con: ${starter ? starter.name : 'Nessuno'}`);
    }

    // =========================================================================
    // Saves Tab (.sav import/export)
    // =========================================================================
    function initSaveManager() {
        document.getElementById('btn-export-sav')?.addEventListener('click', async () => {
            if (window.EJS_emulator && typeof window.EJS_emulator.gameManager?.getState === 'function') {
                const stateData = await window.EJS_emulator.gameManager.getState();
                if (stateData) {
                    const blob = new Blob([stateData], { type: 'application/octet-stream' });
                    const a = document.createElement('a');
                    a.href = URL.createObjectURL(blob);
                    a.download = 'Pokemon_KaizoRogue.sav';
                    a.click();
                    return;
                }
            }
            alert('Salvataggio esportabile dopo aver salvato la partita nel menu di gioco.');
        });

        const btnImp = document.getElementById('btn-import-sav');
        const fileImp = document.getElementById('sav-file-input');
        if (btnImp && fileImp) {
            btnImp.addEventListener('click', () => fileImp.click());
            fileImp.addEventListener('change', async (e) => {
                const file = e.target.files && e.target.files[0];
                if (!file) return;
                const buf = await file.arrayBuffer();
                window.EJS_loadStateURL = URL.createObjectURL(new Blob([buf]));
                alert(`Salvataggio "${file.name}" caricato per la prossima sessione!`);
            });
        }
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();
