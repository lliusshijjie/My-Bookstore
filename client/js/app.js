(function () {
  var SPINE_COLORS = ['--spine-1', '--spine-2', '--spine-3', '--spine-4'];
  var SESSION_KEY = 'booktrade_user';
  var CART_KEY = 'booktrade_cart';

  var state = {
    user: null,
    books: [],
    cart: {},
    authMode: 'login',
    searchQuery: '',
    searchTimer: null
  };

  var els = {};

  function $(id) {
    return document.getElementById(id);
  }

  function formatPrice(cents) {
    return '¥' + (cents / 100).toFixed(2);
  }

  function getSpineColor(id) {
    var idx = (id - 1) % SPINE_COLORS.length;
    return getComputedStyle(document.documentElement).getPropertyValue(SPINE_COLORS[idx]).trim();
  }

  function showToast(msg) {
    els.toast.textContent = msg;
    els.toast.classList.add('show');
    clearTimeout(showToast._t);
    showToast._t = setTimeout(function () {
      els.toast.classList.remove('show');
    }, 2600);
  }

  function setHidden(el, hidden) {
    if (hidden) {
      el.classList.add('hidden');
    } else {
      el.classList.remove('hidden');
    }
  }

  function loadSession() {
    try {
      var raw = sessionStorage.getItem(SESSION_KEY);
      if (raw) {
        state.user = JSON.parse(raw);
      }
    } catch (e) {
      state.user = null;
    }
  }

  function saveSession() {
    if (state.user) {
      sessionStorage.setItem(SESSION_KEY, JSON.stringify(state.user));
    } else {
      sessionStorage.removeItem(SESSION_KEY);
    }
  }

  function loadCart() {
    try {
      var raw = localStorage.getItem(CART_KEY);
      state.cart = raw ? JSON.parse(raw) : {};
    } catch (e) {
      state.cart = {};
    }
  }

  function saveCart() {
    localStorage.setItem(CART_KEY, JSON.stringify(state.cart));
  }

  function cartCount() {
    var n = 0;
    for (var id in state.cart) {
      if (state.cart.hasOwnProperty(id)) {
        n += state.cart[id].quantity;
      }
    }
    return n;
  }

  function cartTotalCents() {
    var total = 0;
    for (var id in state.cart) {
      if (state.cart.hasOwnProperty(id)) {
        var item = state.cart[id];
        total += item.price_cents * item.quantity;
      }
    }
    return total;
  }

  function findBook(id) {
    for (var i = 0; i < state.books.length; i++) {
      if (state.books[i].id === id) {
        return state.books[i];
      }
    }
    return null;
  }

  function updateUserUI() {
    if (state.user) {
      els.userBadge.textContent = state.user.username;
      setHidden(els.userBadge, false);
      setHidden(els.authForm, true);
      setHidden(els.logoutBtn, false);
    } else {
      setHidden(els.userBadge, true);
      setHidden(els.authForm, false);
      setHidden(els.logoutBtn, true);
    }
    els.checkoutBtn.disabled = !state.user || cartCount() === 0;
  }

  function renderCart() {
    var count = cartCount();
    els.cartCount.textContent = String(count);
    els.cartTotal.textContent = formatPrice(cartTotalCents());
    els.checkoutBtn.disabled = !state.user || count === 0;

    els.cartList.innerHTML = '';
    var keys = Object.keys(state.cart);
    setHidden(els.cartEmpty, keys.length > 0);

    keys.forEach(function (id) {
      var item = state.cart[id];
      var li = document.createElement('li');
      li.className = 'cart-item';

      var nameDiv = document.createElement('div');
      nameDiv.className = 'cart-item-name';
      nameDiv.textContent = item.title;

      var priceDiv = document.createElement('div');
      priceDiv.className = 'cart-item-price';
      priceDiv.textContent = formatPrice(item.price_cents);

      var qtyDiv = document.createElement('div');
      qtyDiv.className = 'qty-control';

      var minusBtn = document.createElement('button');
      minusBtn.type = 'button';
      minusBtn.className = 'qty-btn';
      minusBtn.textContent = '−';
      minusBtn.setAttribute('aria-label', '减少数量');
      minusBtn.addEventListener('click', function () {
        changeQty(parseInt(id, 10), -1);
      });

      var qtySpan = document.createElement('span');
      qtySpan.className = 'qty-val';
      qtySpan.textContent = String(item.quantity);

      var plusBtn = document.createElement('button');
      plusBtn.type = 'button';
      plusBtn.className = 'qty-btn';
      plusBtn.textContent = '+';
      plusBtn.setAttribute('aria-label', '增加数量');
      plusBtn.addEventListener('click', function () {
        changeQty(parseInt(id, 10), 1);
      });

      qtyDiv.appendChild(minusBtn);
      qtyDiv.appendChild(qtySpan);
      qtyDiv.appendChild(plusBtn);

      li.appendChild(nameDiv);
      li.appendChild(priceDiv);
      li.appendChild(qtyDiv);
      els.cartList.appendChild(li);
    });
  }

  function changeQty(bookId, delta) {
    var item = state.cart[bookId];
    if (!item) {
      return;
    }
    var book = findBook(bookId);
    var maxStock = book ? book.stock : item.quantity;
    var next = item.quantity + delta;
    if (next <= 0) {
      delete state.cart[bookId];
    } else if (next <= maxStock) {
      item.quantity = next;
    } else {
      showToast('库存不足，最多 ' + maxStock + ' 本');
      return;
    }
    saveCart();
    renderCart();
  }

  function addToCart(book) {
    if (book.status !== 'active') {
      showToast('该图书已下架');
      return;
    }
    var existing = state.cart[book.id];
    if (existing) {
      if (existing.quantity >= book.stock) {
        showToast('库存不足');
        return;
      }
      existing.quantity += 1;
    } else {
      state.cart[book.id] = {
        title: book.title,
        price_cents: book.price_cents,
        quantity: 1
      };
    }
    saveCart();
    renderCart();
    showToast('已加入购物袋');
  }

  function renderBooks(books) {
    var list = books || state.books;
    els.bookGrid.innerHTML = '';
    setHidden(els.catalogEmpty, list.length > 0);
    setHidden(els.catalogError, true);

    list.forEach(function (book, index) {
      var card = document.createElement('article');
      card.className = 'book-card';
      card.setAttribute('role', 'listitem');
      card.style.animationDelay = (index * 0.06) + 's';

      var spine = document.createElement('div');
      spine.className = 'book-spine';
      spine.style.background = getSpineColor(book.id);

      var body = document.createElement('div');
      body.className = 'book-body';

      var title = document.createElement('h3');
      title.className = 'book-title';
      title.textContent = book.title;

      var author = document.createElement('p');
      author.className = 'book-author';
      author.textContent = book.author;

      var meta = document.createElement('div');
      meta.className = 'book-meta';

      var price = document.createElement('span');
      price.className = 'book-price';
      price.textContent = formatPrice(book.price_cents);

      var stock = document.createElement('span');
      stock.className = 'book-stock' + (book.stock <= 3 ? ' low' : '');
      stock.textContent = '库存 ' + book.stock;

      meta.appendChild(price);
      meta.appendChild(stock);

      var addBtn = document.createElement('button');
      addBtn.type = 'button';
      addBtn.className = 'btn btn-primary btn-add';
      addBtn.textContent = '加入购物袋';
      addBtn.disabled = book.stock <= 0 || book.status !== 'active';
      addBtn.addEventListener('click', function () {
        addToCart(book);
      });

      body.appendChild(title);
      body.appendChild(author);
      body.appendChild(meta);
      body.appendChild(addBtn);

      card.appendChild(spine);
      card.appendChild(body);
      els.bookGrid.appendChild(card);
    });
  }

  function renderOrders(orders) {
    els.ordersList.innerHTML = '';
    setHidden(els.ordersEmpty, orders.length > 0);
    setHidden(els.ordersError, true);

    orders.forEach(function (order) {
      var card = document.createElement('div');
      card.className = 'order-card';

      var head = document.createElement('div');
      head.className = 'order-head';

      var idSpan = document.createElement('span');
      idSpan.className = 'order-id';
      idSpan.textContent = '订单 #' + order.id;

      var status = document.createElement('span');
      status.className = 'order-status';
      status.textContent = order.status;

      head.appendChild(idSpan);
      head.appendChild(status);

      var total = document.createElement('p');
      total.className = 'order-total';
      total.textContent = formatPrice(order.total_cents);

      var itemsList = document.createElement('ul');
      itemsList.className = 'order-items';
      (order.items || []).forEach(function (item) {
        var li = document.createElement('li');
        var book = findBook(item.book_id);
        var name = book ? book.title : '图书 #' + item.book_id;
        li.textContent = name + ' × ' + item.quantity;
        itemsList.appendChild(li);
      });

      card.appendChild(head);
      card.appendChild(total);
      if (order.items && order.items.length) {
        card.appendChild(itemsList);
      }
      els.ordersList.appendChild(card);
    });
  }

  function switchTab(tabName) {
    var tabs = document.querySelectorAll('.tab');
    var panels = {
      auth: $('tab-auth'),
      cart: $('tab-cart'),
      orders: $('tab-orders')
    };

    tabs.forEach(function (tab) {
      var active = tab.getAttribute('data-tab') === tabName;
      tab.classList.toggle('active', active);
      tab.setAttribute('aria-selected', active ? 'true' : 'false');
    });

    for (var key in panels) {
      if (panels.hasOwnProperty(key)) {
        var panel = panels[key];
        var show = key === tabName;
        panel.classList.toggle('active', show);
        if (show) {
          panel.removeAttribute('hidden');
        } else {
          panel.setAttribute('hidden', '');
        }
      }
    }

    if (tabName === 'orders') {
      fetchOrders();
    }
  }

  function setAuthMode(mode) {
    state.authMode = mode;
    document.querySelectorAll('.auth-mode').forEach(function (btn) {
      btn.classList.toggle('active', btn.getAttribute('data-mode') === mode);
    });
    els.authSubmit.textContent = mode === 'login' ? '登录' : '注册';
    var passInput = els.authForm.querySelector('input[name="password"]');
    passInput.autocomplete = mode === 'login' ? 'current-password' : 'new-password';
    setHidden(els.authError, true);
  }

  function checkHealth() {
    Api.health().then(function (res) {
      if (res.ok && res.code === 0) {
        els.gatewayStatus.textContent = '网关在线';
        els.gatewayStatus.setAttribute('data-state', 'ok');
      } else {
        els.gatewayStatus.textContent = '网关异常';
        els.gatewayStatus.setAttribute('data-state', 'error');
      }
    }).catch(function () {
      els.gatewayStatus.textContent = '无法连接';
      els.gatewayStatus.setAttribute('data-state', 'error');
    });
  }

  function setSearchStatus(text, status) {
    if (!text) {
      setHidden(els.searchStatus, true);
      els.searchStatus.textContent = '';
      els.searchStatus.removeAttribute('data-state');
      return;
    }
    els.searchStatus.textContent = text;
    els.searchStatus.setAttribute('data-state', status || '');
    setHidden(els.searchStatus, false);
  }

  function updateSearchUI() {
    var hasQuery = state.searchQuery.length > 0;
    setHidden(els.searchClear, !hasQuery);
    els.catalogHeading.textContent = hasQuery ? '搜索结果' : '书目';
  }

  function fetchBooks() {
    Api.books().then(function (res) {
      if (res.ok && res.data && res.data.books) {
        state.books = res.data.books;
        if (!state.searchQuery) {
          renderBooks(state.books);
          setSearchStatus('', '');
        }
      } else {
        els.catalogError.textContent = res.message || '加载书目失败';
        setHidden(els.catalogError, false);
      }
    }).catch(function () {
      els.catalogError.textContent = '无法加载书目，请确认服务已启动';
      setHidden(els.catalogError, false);
    });
  }

  function runSearch(query) {
    state.searchQuery = query.trim();
    updateSearchUI();

    if (!state.searchQuery) {
      setSearchStatus('', '');
      renderBooks(state.books);
      return;
    }

    setSearchStatus('搜索中…', 'loading');
    Api.searchBooks(state.searchQuery).then(function (res) {
      if (res.ok && res.data && res.data.books) {
        var engine = res.data.engine || 'memory';
        var count = res.data.books.length;
        setSearchStatus(
          '找到 ' + count + ' 本 · 引擎 ' + engine,
          ''
        );
        renderBooks(res.data.books);
      } else {
        setSearchStatus(res.message || '搜索失败', 'error');
        renderBooks([]);
      }
    }).catch(function () {
      setSearchStatus('搜索请求失败', 'error');
      renderBooks([]);
    });
  }

  function handleSearchInput() {
    var query = els.searchInput.value;
    clearTimeout(state.searchTimer);
    state.searchTimer = setTimeout(function () {
      runSearch(query);
    }, 300);
  }

  function fetchOrders() {
    Api.listOrders().then(function (res) {
      if (res.ok && res.data && res.data.orders) {
        renderOrders(res.data.orders);
      } else {
        els.ordersError.textContent = res.message || '加载订单失败';
        setHidden(els.ordersError, false);
      }
    }).catch(function () {
      els.ordersError.textContent = '无法加载订单';
      setHidden(els.ordersError, false);
    });
  }

  function handleAuthSubmit(e) {
    e.preventDefault();
    setHidden(els.authError, true);

    var form = els.authForm;
    var username = form.username.value.trim();
    var password = form.password.value;

    if (!username || !password) {
      els.authError.textContent = '请填写用户名和密码';
      setHidden(els.authError, false);
      return;
    }

    var call = state.authMode === 'login'
      ? Api.login(username, password)
      : Api.register(username, password);

    call.then(function (res) {
      if (res.ok && res.data) {
        state.user = { user_id: res.data.user_id, username: res.data.username };
        saveSession();
        updateUserUI();
        form.reset();
        showToast(state.authMode === 'login' ? '登录成功' : '注册成功');
      } else {
        els.authError.textContent = res.message || '操作失败';
        setHidden(els.authError, false);
      }
    }).catch(function () {
      els.authError.textContent = '网络错误，请重试';
      setHidden(els.authError, false);
    });
  }

  function handleCheckout() {
    setHidden(els.checkoutError, true);

    if (!state.user) {
      switchTab('auth');
      showToast('请先登录');
      return;
    }

    var items = [];
    for (var id in state.cart) {
      if (state.cart.hasOwnProperty(id)) {
        items.push({ book_id: parseInt(id, 10), quantity: state.cart[id].quantity });
      }
    }

    if (!items.length) {
      return;
    }

    els.checkoutBtn.disabled = true;

    Api.createOrder(state.user.user_id, items).then(function (res) {
      els.checkoutBtn.disabled = false;
      if (res.ok && res.data && res.data.order) {
        state.cart = {};
        saveCart();
        renderCart();
        fetchBooks();
        showToast('订单 #' + res.data.order.id + ' 已创建');
        switchTab('orders');
      } else {
        els.checkoutError.textContent = res.message || '下单失败';
        setHidden(els.checkoutError, false);
      }
    }).catch(function () {
      els.checkoutBtn.disabled = false;
      els.checkoutError.textContent = '网络错误，请重试';
      setHidden(els.checkoutError, false);
    });
  }

  function bindEvents() {
    document.querySelectorAll('.tab').forEach(function (tab) {
      tab.addEventListener('click', function () {
        switchTab(tab.getAttribute('data-tab'));
      });
    });

    document.querySelectorAll('.auth-mode').forEach(function (btn) {
      btn.addEventListener('click', function () {
        setAuthMode(btn.getAttribute('data-mode'));
      });
    });

    els.authForm.addEventListener('submit', handleAuthSubmit);
    els.logoutBtn.addEventListener('click', function () {
      state.user = null;
      saveSession();
      updateUserUI();
      showToast('已退出登录');
    });
    els.checkoutBtn.addEventListener('click', handleCheckout);
    els.refreshOrders.addEventListener('click', fetchOrders);
    els.searchForm.addEventListener('submit', function (e) {
      e.preventDefault();
      clearTimeout(state.searchTimer);
      runSearch(els.searchInput.value);
    });
    els.searchInput.addEventListener('input', handleSearchInput);
    els.searchClear.addEventListener('click', function () {
      els.searchInput.value = '';
      clearTimeout(state.searchTimer);
      runSearch('');
      els.searchInput.focus();
    });
  }

  function init() {
    els = {
      gatewayStatus: $('gateway-status'),
      userBadge: $('user-badge'),
      catalogHeading: $('catalog-heading'),
      searchForm: $('search-form'),
      searchInput: $('search-input'),
      searchClear: $('search-clear'),
      searchStatus: $('search-status'),
      bookGrid: $('book-grid'),
      catalogEmpty: $('catalog-empty'),
      catalogError: $('catalog-error'),
      cartCount: $('cart-count'),
      cartList: $('cart-list'),
      cartEmpty: $('cart-empty'),
      cartTotal: $('cart-total'),
      checkoutBtn: $('checkout-btn'),
      checkoutError: $('checkout-error'),
      authForm: $('auth-form'),
      authSubmit: $('auth-submit'),
      authError: $('auth-error'),
      logoutBtn: $('logout-btn'),
      ordersList: $('orders-list'),
      ordersEmpty: $('orders-empty'),
      ordersError: $('orders-error'),
      refreshOrders: $('refresh-orders'),
      toast: $('toast')
    };

    loadSession();
    loadCart();
    bindEvents();
    updateUserUI();
    renderCart();
    checkHealth();
    fetchBooks();
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    init();
  }
})();
