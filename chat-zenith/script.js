document.addEventListener('DOMContentLoaded', function() {
    // 配置marked.js - 用于渲染Markdown格式的回复
    marked.setOptions({
        highlight: function(code, lang) {
            const language = hljs.getLanguage(lang) ? lang : 'plaintext';
            return hljs.highlight(code, { language }).value;
        },
        breaks: true,
        gfm: true
    });

    const chatHistory = document.getElementById('chat-history');
    const userInput = document.getElementById('user-input');
    const sendBtn = document.getElementById('send-btn');
    const clearBtn = document.getElementById('clear-btn');
    
    // ===== 主要修改部分：更换API端点 =====
    // 原先的Cloudflare Worker地址已被替换为阿里云服务器的Nginx反向代理地址
    // 使用HTTP协议，因为博客可能是HTTPS，这里需要根据实际情况调整
    const API_URL = 'http://39.103.58.123/api/langchain/chat';
    
    // 如果你的博客是HTTPS，浏览器可能会阻止HTTP请求（混合内容问题）
    // 如果遇到这个问题，有两个解决方案：
    // 1. 为阿里云服务器配置HTTPS（推荐，后续步骤会讲）
    // 2. 临时方案：如果博客支持HTTP访问，可以先用HTTP访问博客
    
    // 对话历史记录，用于保持上下文
    let conversationHistory = [];
    
    /**
     * 发送消息的主函数
     * 处理用户输入，调用API，显示响应
     */
    async function sendMessage() {
        const message = userInput.value.trim();
        if (!message) return;
        
        // 禁用输入控件，防止重复发送
        sendBtn.disabled = true;
        userInput.disabled = true;
        
        // 将用户消息添加到聊天界面
        addMessageToChat('user', message);
        userInput.value = '';
        
        // 显示AI正在思考的提示
        const thinkingMsgId = showThinkingMessage();
        
        try {
            // ===== 修改部分：调用新的API =====
            const response = await callLangchainAPI(message);
            
            // 移除思考提示
            removeThinkingMessage(thinkingMsgId);
            
            // 处理响应 - 响应格式与原先相同，无需修改
            if (response && response.choices && response.choices[0].message.content) {
                const aiResponse = response.choices[0].message.content;
                addMessageToChat('assistant', aiResponse, true); // 标记为需要渲染Markdown
                
                // 更新对话历史
                conversationHistory.push(
                    { role: 'user', content: message },
                    { role: 'assistant', content: aiResponse }
                );
            } else {
                throw new Error('无效的API响应');
            }
        } catch (error) {
            removeThinkingMessage(thinkingMsgId);
            console.error('API调用失败:', error);
            
            // ===== 新增：更详细的错误提示 =====
            let errorMessage = '抱歉，出现错误: ';
            if (error.message.includes('Failed to fetch')) {
                errorMessage += '无法连接到服务器，请检查网络连接';
            } else if (error.message.includes('Mixed Content')) {
                errorMessage += 'HTTPS/HTTP混合内容问题，请联系管理员';
            } else {
                errorMessage += error.message;
            }
            addMessageToChat('assistant', errorMessage);
        } finally {
            // 恢复输入控件
            sendBtn.disabled = false;
            userInput.disabled = false;
            userInput.focus();
        }
    }
    
    /**
     * 调用Langchain API的函数
     * 替换原先的callDeepSeekAPI函数
     * @param {string} message - 用户输入的消息
     * @returns {Promise} API响应
     */
    async function callLangchainAPI(message) {
        try {
            // 发送POST请求到新的API端点
            const response = await fetch(API_URL, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    // 如果后续添加了API密钥验证，可以在这里添加
                    // 'Authorization': 'Bearer YOUR_API_KEY'
                },
                body: JSON.stringify({
                    model: 'langchain-deepseek',  // 可选，用于标识
                    messages: [
                        ...conversationHistory,    // 包含历史对话
                        { role: 'user', content: message }
                    ]
                })
            });
            
            // 检查响应状态
            if (!response.ok) {
                const errorData = await response.json();
                throw new Error(errorData.error || `请求失败: ${response.status}`);
            }
            
            // 返回JSON响应
            return response.json();
        } catch (error) {
            console.error('Fetch错误:', error);
            
            // 提供更友好的错误信息
            if (error instanceof TypeError && error.message === 'Failed to fetch') {
                throw new Error('无法连接到AI服务，请稍后重试');
            }
            throw new Error(`服务连接异常: ${error.message}`);
        }
    }
    
    // ===== 以下函数保持不变 =====
    
    /**
     * 添加消息到聊天界面
     * 支持Markdown渲染，AI消息统一样式
     */
    function addMessageToChat(role, content, isMarkdown = false) {
        const messageDiv = document.createElement('div');
        if (role === 'assistant') {
            messageDiv.className = 'message ai-message';
            messageDiv.style.display = 'flex';
            messageDiv.style.alignItems = 'flex-start';

            const avatar = document.createElement('img');
            avatar.src = 'https://smallgoodgood.top/images/23.jpg';
            avatar.className = 'cat-avatar';
            messageDiv.appendChild(avatar);

            const contentDiv = document.createElement('div');
            contentDiv.style.flex = '1';
            if (isMarkdown) {
                // 支持 \\( ... \\) 行内公式和 \\[ ... \\] 块公式
                let mathContent = content
                    .replace(/\\\((.+?)\\\)/g, function(match, p1) {
                        return '$' + p1 + '$';
                    })
                    .replace(/\\\[(.+?)\\\]/g, function(match, p1) {
                        return '$$' + p1 + '$$';
                    });
                contentDiv.innerHTML = marked.parse(mathContent);
            } else {
                contentDiv.textContent = content;
            }
            messageDiv.appendChild(contentDiv);
        } else {
            messageDiv.className = 'message user-message';
            const messageContent = document.createElement('div');
            messageContent.textContent = content;
            messageDiv.appendChild(messageContent);
        }

        const messageTime = document.createElement('div');
        messageTime.className = 'message-time';
        messageTime.textContent = new Date().toLocaleTimeString();
        messageDiv.appendChild(messageTime);

        chatHistory.appendChild(messageDiv);
        chatHistory.scrollTop = chatHistory.scrollHeight;

        // 高亮代码块
        if (isMarkdown && role === 'assistant') {
            messageDiv.querySelectorAll('pre code').forEach((block) => {
                hljs.highlightElement(block);
            });
        }

        // 渲染数学公式（MathJax）
        if (window.MathJax && typeof MathJax.typesetPromise === 'function') {
            MathJax.typesetPromise([messageDiv]);
        }
    }

    /**
     * 显示AI思考中的动画提示
     */
    function showThinkingMessage() {
        const messageDiv = document.createElement('div');
        messageDiv.className = 'message ai-message';
        messageDiv.id = 'thinking-message';

        const innerDiv = document.createElement('div');
        innerDiv.style.display = 'flex';
        innerDiv.style.alignItems = 'flex-start';

        const avatar = document.createElement('img');
        avatar.src = 'https://smallgoodgood.top/images/23.jpg';
        avatar.className = 'cat-avatar';
        innerDiv.appendChild(avatar);

        const contentDiv = document.createElement('div');
        const thinkingText = document.createElement('span');
        thinkingText.textContent = '正在思考...';
        contentDiv.appendChild(thinkingText);
        const loadingSpan = document.createElement('span');
        loadingSpan.className = 'loading';
        contentDiv.appendChild(loadingSpan);
        innerDiv.appendChild(contentDiv);

        messageDiv.appendChild(innerDiv);

        const messageTime = document.createElement('div');
        messageTime.className = 'message-time';
        messageTime.textContent = new Date().toLocaleTimeString();
        messageDiv.appendChild(messageTime);

        chatHistory.appendChild(messageDiv);
        chatHistory.scrollTop = chatHistory.scrollHeight;

        return 'thinking-message';
    }
    
    /**
     * 移除思考提示
     */
    function removeThinkingMessage(messageId) {
        const thinkingMsg = document.getElementById(messageId);
        if (thinkingMsg) {
            thinkingMsg.remove();
        }
    }
    
    /**
     * 清空聊天历史
     */
    function clearChatHistory() {
        // 保留欢迎消息
        const welcomeMessage = chatHistory.querySelector('.ai-message');
        chatHistory.innerHTML = '';
        if (welcomeMessage) {
            chatHistory.appendChild(welcomeMessage.cloneNode(true));
        }
        conversationHistory = [];
    }
    
    // ===== 事件监听器（保持不变）=====
    sendBtn.addEventListener('click', sendMessage);
    
    userInput.addEventListener('keydown', function(e) {
        if (e.key === 'Enter' && !e.shiftKey) {
            e.preventDefault();
            sendMessage();
        }
    });
    
    clearBtn.addEventListener('click', clearChatHistory);
    
    // 限制输入字数为500
    userInput.setAttribute('maxlength', '500');

    // 字数统计提示（左下角）
    let charCountTip = document.createElement('div');
    charCountTip.style.position = 'absolute';
    charCountTip.style.left = '16px';
    charCountTip.style.bottom = '12px';
    charCountTip.style.fontSize = '12px';
    charCountTip.style.color = '#888';
    charCountTip.style.zIndex = '2';
    userInput.parentNode.appendChild(charCountTip);

    function updateCharCount() {
        const len = userInput.value.length;
        charCountTip.textContent = `${len}/500`;
        if (len >= 500) {
            charCountTip.style.color = '#e74c3c';
        } else {
            charCountTip.style.color = '#888';
        }
    }
    userInput.addEventListener('input', updateCharCount);
    updateCharCount();

    // 自动聚焦到输入框
    userInput.focus();
});