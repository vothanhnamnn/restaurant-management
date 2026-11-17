// Uses an isolated Chrome/Edge profile; never touches the user's restaurant data.
const { existsSync, mkdirSync, writeFileSync } = require('node:fs');
const { resolve, join } = require('node:path');
const { pathToFileURL } = require('node:url');
const { spawnSync } = require('node:child_process');
const assert = require('node:assert/strict');

const web = resolve(__dirname, '..');
const build = join(web, 'build');
mkdirSync(build, { recursive: true });
const browser = process.env.BROWSER_PATH || [
  'C:/Program Files/Google/Chrome/Application/chrome.exe',
  'C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe',
].find(existsSync);
assert(browser, 'Set BROWSER_PATH to a Chrome/Edge executable.');

function browserChecks() {
  const frame = document.querySelector('iframe');
  frame.onload = async () => {
    const w = frame.contentWindow, d = w.document;
    const results = [];
    const evaluate = code => w.eval(code);
    const check = (condition, message) => { if (!condition) throw Error(message); };
    const click = selector => {
      const button = d.querySelector(selector);
      check(button, 'Missing control: ' + selector);
      button.click();
    };
    const submit = (id, values) => {
      const form = d.getElementById(id);
      check(form, 'Missing form: ' + id);
      for (const [key, value] of Object.entries(values)) form.elements.namedItem(key).value = value;
      form.dispatchEvent(new w.Event('submit', { bubbles: true, cancelable: true }));
    };
    const input = (id, value) => {
      const el = d.getElementById(id);
      el.value = value;
      el.dispatchEvent(new w.Event('input', { bubbles: true }));
    };
    const test = (name, fn) => { fn(); results.push(name); };
    try {
      w.confirm = () => true;
      w.alert = message => { throw Error(message); };
      test('Initial render and stylesheet', () => {
        check(evaluate('state.tables.length === 12 && state.menu.length === 14'), 'Seed data');
        check(w.getComputedStyle(d.querySelector('.sidebar')).backgroundColor === 'rgb(255, 255, 255)', 'Light sidebar CSS did not load');
        check(w.getComputedStyle(d.querySelector('.btn')).transitionDuration === '0s', 'Unexpected animation');
      });
      test('Table create, edit, search and delete', () => {
        click('[data-route="tables"]'); click('[data-action="add-table"]');
        submit('tableForm', { name: 'Test table', capacity: '6' });
        const id = evaluate('state.tables.at(-1).id');
        input('tableSearch', 'Test table');
        check(d.querySelectorAll('.table-card').length === 1, 'Table search');
        click(`[data-edit-table="${id}"]`);
        submit('tableForm', { name: 'Edited table', capacity: '4' });
        input('tableSearch', 'Edited table'); click(`[data-edit-table="${id}"]`);
        click(`[data-delete-table="${id}"]`);
        check(evaluate('state.tables.length === 12'), 'Table deletion');
        input('tableSearch', '');
      });
      test('Menu create, edit, availability, category, search and delete', () => {
        click('[data-route="menu"]'); click('[data-action="add-dish"]');
        submit('dishForm', { name: 'Test dish', category: 'Khai vị', price: '50000', emoji: '*' });
        const id = evaluate('state.menu.at(-1).id');
        input('menuSearch', 'Test dish');
        check(d.querySelectorAll('.menu-card').length === 1, 'Menu search');
        click(`[data-edit-dish="${id}"]`);
        submit('dishForm', { name: 'Edited dish', category: 'Đồ uống', price: '45000', emoji: '*' });
        input('menuSearch', 'Edited dish');
        click(`[data-toggle-dish="${id}"]`);
        check(evaluate('state.menu.at(-1).available === false'), 'Dish availability');
        click('[data-menu-filter="Đồ uống"]');
        check(d.querySelectorAll('.menu-card').length === 1, 'Category filter');
        click(`[data-edit-dish="${id}"]`); click(`[data-delete-dish="${id}"]`);
        check(evaluate('state.menu.length === 14'), 'Dish deletion');
        input('menuSearch', ''); click('[data-menu-filter="Tất cả"]');
      });
      test('Order creation, quantity, busy filter and payment exclusion in reports', () => {
        click('[data-route="tables"]'); click('[data-open-table="table_1"]');
        click('[data-add-to-order="dish_1"]'); click('[data-add-to-order="dish_1"]');
        click('[data-qty="dish_1"][data-delta="-1"]');
        click('[data-add-to-order="dish_2"]');
        check(evaluate('orderAmount(openOrder("table_1")) === 130000'), 'Order amount');
        click('[data-action="close-drawer"]'); click('[data-table-filter="busy"]');
        check(d.querySelectorAll('.table-card').length === 1, 'Busy filter');
        click('[data-route="reports"]');
        check(evaluate('allPaid().length === 0'), 'Unpaid order counted');
      });
      test('Cash checkout, receipt, free table and report totals', () => {
        click('[data-route="orders"]'); click('[data-view-order]');
        click('[data-action="pay-order"]'); submit('paymentForm', { method: 'cash' });
        check(evaluate('!openOrder("table_1") && allPaid()[0].paymentMethod === "cash"'), 'Checkout state');
        check(d.getElementById('receiptView').textContent.includes('130.000'), 'Receipt total');
        click('[data-action="close-modal"]'); click('[data-route="reports"]');
        check(d.querySelector('.stat-value').textContent.includes('130.000'), 'Revenue');
        check(d.querySelectorAll('.best-seller').length === 2, 'Best sellers');
      });
      test('Bank checkout and cancellation', () => {
        evaluate('showDrawer("table_2")'); click('[data-add-to-order="dish_3"]');
        click('[data-action="pay-order"]'); submit('paymentForm', { method: 'bank' });
        check(evaluate('allPaid().length === 2 && allPaid()[1].paymentMethod === "bank"'), 'Bank checkout');
        click('[data-action="close-modal"]'); evaluate('showDrawer("table_3")');
        click('[data-add-to-order="dish_1"]'); click('[data-action="cancel-order"]');
        check(evaluate('!openOrder("table_3") && state.orders.length === 2'), 'Order cancellation');
        click('[data-action="close-drawer"]');
      });
      test('JSON serialization and persistence', () => {
        check(evaluate('validData(JSON.parse(localStorage.getItem(KEY)))'), 'Stored data format');
        check(evaluate('JSON.stringify(load()) === JSON.stringify(state)'), 'Reloaded state mismatch');
      });
      for (const width of [1440, 768, 390, 320]) {
        frame.style.width = width + 'px';
        for (const route of ['overview', 'tables', 'menu', 'orders', 'reports']) {
          evaluate(`setRoute('${route}')`);
          const outside = [...d.querySelectorAll('body *')].filter(el => {
            const rect = el.getBoundingClientRect();
            return rect.width && rect.right > width + 1 && w.getComputedStyle(el).visibility !== 'hidden';
          }).slice(0, 8).map(el => el.className || el.tagName).join(', ');
          check(d.documentElement.scrollWidth <= width, `Page overflow: ${route} at ${width}px (${d.documentElement.scrollWidth}px; ${outside})`);
        }
        evaluate('showDrawer("table_1")');
        const drawer = d.getElementById('drawer');
        check(drawer.scrollWidth <= drawer.clientWidth, `Order drawer overflow at ${width}px`);
        check(d.querySelector('[data-add-to-order="dish_1"]'), 'Order controls missing');
        evaluate('closeDrawer()');
        results.push('All five screens fit ' + width + 'px');
      }
      document.getElementById('result').textContent = 'PASS\n' + results.join('\n');
    } catch (error) {
      document.getElementById('result').textContent = 'FAIL\n' + error.stack;
    }
  };
  frame.src = document.body.dataset.app;
}
const harness = join(build, 'smoke.html');
writeFileSync(harness, `<!doctype html><meta charset="utf-8"><body data-app="${pathToFileURL(join(web, 'index.html'))}"><pre id="result">RUNNING</pre><iframe style="width:1440px;height:1000px;border:0"></iframe><script>(${browserChecks.toString()})()</script>`);
const result = spawnSync(browser, [
  '--headless', '--disable-gpu', '--no-first-run', '--no-default-browser-check',
  '--allow-file-access-from-files', '--disable-extensions',
  '--user-data-dir=' + join(build, 'profile-' + Date.now()),
  '--dump-dom', '--virtual-time-budget=8000', pathToFileURL(harness).href,
], { encoding: 'utf8', timeout: 30000, maxBuffer: 4 * 1024 * 1024 });
const output = result.stdout?.match(/<pre id="result">([\s\S]*?)<\/pre>/)?.[1];
console.log(output || result.stderr || result.error?.message || 'No browser output');
assert(/^PASS\r?\n/.test(output || ''), 'Browser smoke checks failed');
