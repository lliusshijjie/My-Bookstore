var Api = (function () {
  function request(method, path, body) {
    var opts = {
      method: method,
      headers: { 'Content-Type': 'application/json' }
    };
    if (body) {
      opts.body = JSON.stringify(body);
    }
    return fetch(path, opts).then(function (res) {
      return res.json().then(function (data) {
        return {
          ok: res.ok,
          status: res.status,
          code: data.code,
          message: data.message,
          data: data.data
        };
      });
    });
  }

  return {
    health: function () { return request('GET', '/api/health'); },
    register: function (username, password) {
      return request('POST', '/api/auth/register', { username: username, password: password });
    },
    login: function (username, password) {
      return request('POST', '/api/auth/login', { username: username, password: password });
    },
    books: function () { return request('GET', '/api/books'); },
    inventory: function (bookId) {
      return request('GET', '/api/inventory/books/' + bookId);
    },
    createOrder: function (userId, items) {
      return request('POST', '/api/orders', { user_id: userId, items: items });
    },
    listOrders: function () { return request('GET', '/api/orders'); }
  };
})();
